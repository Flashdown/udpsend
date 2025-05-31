// udpsend v0.8 Copyright (C) 2024 Enrico Heine https://github.com/Flashdown/udpsend

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 3 as
// published by the Free Software Foundation Version 3 of the License.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <limits>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

const size_t MAX_UDP_MESSAGE_SIZE = 65507;
const size_t MAX_SERVER_LENGTH = 200;
const size_t MAX_PORT_LENGTH = 5;

static void printUsage(const char* progName) {
    std::cerr << std::endl << " udpsend v0.8 Copyright (C) 2024 Enrico Heine" << std::endl << std::endl;
    std::cerr << " https://github.com/Flashdown/udpsend" << std::endl << std::endl;
    std::cerr << " This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cerr << " This is free software, and you are welcome to redistribute it" << std::endl;
    std::cerr << " under the conditions of the GNU General Public License Version 3" << std::endl << std::endl;
    std::cerr << "Usage: " << progName << " [-4 | -6] [-h] <server> <port> [<message>]" << std::endl;
    std::cerr << "  -4: Force IPv4" << std::endl;
    std::cerr << "  -6: Force IPv6" << std::endl;
    std::cerr << "  -h: Input message as HEX string" << std::endl;
    std::cerr << "  server: IPv4, IPv6 address or FQDN" << std::endl;
    std::cerr << "  port:   Port number to send the message to (1-65535)" << std::endl;
    std::cerr << "  message: Message to send via UDP (max 65507 bytes)" << std::endl;
}

static bool isValidPort(const std::string& portStr) {
    std::regex portRegex("^[0-9]+$");
    if (!std::regex_match(portStr, portRegex)) {
        return false;
    }
    try {
        long port = std::stol(portStr);
        return port >= 1 && port <= 65535;
    }
    catch (const std::exception&) {
        return false;
    }
}

static bool isValidIPv4(const std::string& server) {
    std::regex ipv4SimpleRegex("^[0-9.]+$");
    return std::regex_match(server, ipv4SimpleRegex);
}

static bool isValidIPv6(const std::string& server) {
    std::regex ipv6Regex("^[0-9a-fA-F:]+$");
    return std::regex_match(server, ipv6Regex);
}

static bool isValidDomain(const std::string& server) {
    std::regex domainRegex("^([a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}$");
    return std::regex_match(server, domainRegex);
}

static bool isValidServer(const std::string& server) {
    return isValidIPv4(server) || isValidIPv6(server) || isValidDomain(server);
}

static bool isValidMessage(const std::string& message) {
    for (size_t i = 0; i < message.length();) {
        unsigned char c = message[i];
        if (c <= 0x7F) {
            ++i;
        }
        else if ((c & 0xE0) == 0xC0 && i + 1 < message.length()) {
            ++i; ++i;
        }
        else if ((c & 0xF0) == 0xE0 && i + 2 < message.length()) {
            ++i; ++i; ++i;
        }
        else if ((c & 0xF8) == 0xF0 && i + 3 < message.length()) {
            ++i; ++i; ++i; ++i;
        }
        else {
            return false;
        }
    }
    return true;
}

static bool isValidHex(const std::string& hex) {
    std::regex hexRegex("^[0-9a-fA-F]*$");
    return std::regex_match(hex, hexRegex) && (hex.length() % 2 == 0);
}

static std::string hexToString(const std::string& hex) {
    std::string result;
    result.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte = hex.substr(i, 2);
        char chr = static_cast<char>(std::stoi(byte, nullptr, 16));
        result.push_back(chr);
    }
    return result;
}

#ifdef _WIN32
static std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
    return str;
}
#endif

static int determineAddressFamily(const std::string& server, const std::string& port) {
    struct addrinfo hints, * res, * p;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow both IPv4 and IPv6

    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &res) != 0) {
        return AF_UNSPEC;
    }

    // Prioritize AF_INET (IPv4) over AF_INET6 (IPv6)
    int selected_family = AF_UNSPEC;
    for (p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            selected_family = AF_INET;
            break; // Prefer IPv4
        }
        else if (p->ai_family == AF_INET6 && selected_family != AF_INET) {
            selected_family = AF_INET6; // Use IPv6 only if no IPv4 found
        }
    }

    freeaddrinfo(res);
    return selected_family;
}

int main(int argc, char* argv[]) {
    std::string server;
    long port = 0;
    std::string message;
    int ai_family = AF_UNSPEC;
    bool useHex = false;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return EXIT_FAILURE;
    }
#endif

    std::vector<std::string> utf8_argv;
#ifdef _WIN32
    int wargc;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (!wargv) {
        std::cerr << "Failed to get command-line arguments" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }
    utf8_argv.resize(wargc);
    for (int i = 0; i < wargc; ++i) {
        utf8_argv[i] = wideToUtf8(wargv[i]);
    }
    LocalFree(wargv);
#else
    utf8_argv.assign(argv, argv + argc);
#endif

    try {
        int arg_offset = 0;
        for (int i = 1; i < argc && utf8_argv[i][0] == '-'; ++i) {
            if (utf8_argv[i] == "-4") {
                ai_family = AF_INET;
                arg_offset++;
            }
            else if (utf8_argv[i] == "-6") {
                ai_family = AF_INET6;
                arg_offset++;
            }
            else if (utf8_argv[i] == "-h") {
                useHex = true;
                arg_offset++;
            }
            else {
                throw std::invalid_argument("Unknown option: " + utf8_argv[i]);
            }
        }

        if (argc == 4 + arg_offset) {
            server = utf8_argv[1 + arg_offset];
            if (server.length() > MAX_SERVER_LENGTH) {
                throw std::invalid_argument("Server address is too long.");
            }
            if (!isValidServer(server)) {
                throw std::invalid_argument("Invalid server address or domain name.");
            }

            if (utf8_argv[2 + arg_offset].length() > MAX_PORT_LENGTH) {
                throw std::invalid_argument("Port number is too long.");
            }
            if (!isValidPort(utf8_argv[2 + arg_offset])) {
                throw std::invalid_argument("Invalid port number. Must be between 1 and 65535.");
            }
            port = std::stol(utf8_argv[2 + arg_offset]);

            message = utf8_argv[3 + arg_offset];
            if (useHex) {
                if (!isValidHex(message)) {
                    throw std::invalid_argument("Invalid HEX string. Must be valid hexadecimal with even length.");
                }
                message = hexToString(message);
            }
            else {
                if (!isValidMessage(message)) {
                    throw std::invalid_argument("Message contains invalid characters.");
                }
            }
            if (message.length() > MAX_UDP_MESSAGE_SIZE) {
                throw std::invalid_argument("Message is too long for a single UDP packet.");
            }
        }
        else if (argc == 3 + arg_offset) {
            server = utf8_argv[1 + arg_offset];
            if (server.length() > MAX_SERVER_LENGTH) {
                throw std::invalid_argument("Server address is too long.");
            }
            if (!isValidServer(server)) {
                throw std::invalid_argument("Invalid server address or domain name.");
            }

            if (utf8_argv[2 + arg_offset].length() > MAX_PORT_LENGTH) {
                throw std::invalid_argument("Port number is too long.");
            }
            if (!isValidPort(utf8_argv[2 + arg_offset])) {
                throw std::invalid_argument("Invalid port number. Must be between 1 and 65535.");
            }
            port = std::stol(utf8_argv[2 + arg_offset]);

#ifdef _WIN32
            std::vector<wchar_t> wbuffer(MAX_UDP_MESSAGE_SIZE + 1);
            if (useHex) {
                std::cout << "Enter the HEX message to send: ";
            }
            else {
                std::cout << "Enter the message to send: ";
            }
            DWORD charsRead = 0;
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            if (!ReadConsoleW(hStdin, wbuffer.data(), static_cast<DWORD>(MAX_UDP_MESSAGE_SIZE), &charsRead, NULL)) {
                throw std::runtime_error("Failed to read console input.");
            }
            if (charsRead > 0 && wbuffer[charsRead - 1] == L'\n') --charsRead;
            if (charsRead > 0 && wbuffer[charsRead - 1] == L'\r') --charsRead;
            wbuffer[charsRead] = L'\0';
            std::wstring wmessage(wbuffer.data(), charsRead);
            message = wideToUtf8(wmessage);
#else
            if (useHex) {
                std::cout << "Enter the HEX message to send: ";
            }
            else {
                std::cout << "Enter the message to send: ";
            }
            std::getline(std::cin, message);
#endif

            if (message.empty()) {
                throw std::invalid_argument("Message cannot be empty.");
            }
            if (useHex) {
                if (!isValidHex(message)) {
                    throw std::invalid_argument("Invalid HEX string. Must be valid hexadecimal with even length.");
                }
                message = hexToString(message);
            }
            else {
                if (!isValidMessage(message)) {
                    throw std::invalid_argument("Message contains invalid characters.");
                }
            }
            if (message.length() > MAX_UDP_MESSAGE_SIZE) {
                throw std::invalid_argument("Message is too long for a single UDP packet.");
            }
        }
        else {
            printUsage(utf8_argv[0].c_str());
            return EXIT_FAILURE;
        }

        // Determine address family if not specified by -4 or -6
        if (ai_family == AF_UNSPEC) {
            if (isValidIPv4(server)) {
                ai_family = AF_INET;
            }
            else if (isValidIPv6(server)) {
                ai_family = AF_INET6;
            }
            else if (isValidDomain(server)) {
                ai_family = determineAddressFamily(server, std::to_string(port));
                if (ai_family == AF_UNSPEC) {
                    throw std::invalid_argument("Unable to resolve domain name to a supported address family.");
                }
            }
            else {
                throw std::invalid_argument("Invalid server address or domain name.");
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    SOCKET sockfd = socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }
#else
    int sockfd = socket(ai_family, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }
#endif

    struct addrinfo hints, * res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
#ifdef _WIN32
        std::cerr << "Failed to resolve server address: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
#else
        perror("Failed to resolve server address");
        close(sockfd);
#endif
        return EXIT_FAILURE;
    }

    int sent = sendto(sockfd, message.c_str(), static_cast<int>(message.length()), 0, res->ai_addr, static_cast<int>(res->ai_addrlen));
    if (sent == -1) {
#ifdef _WIN32
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
#else
        perror("Failed to send message");
        close(sockfd);
#endif
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    std::cout << "Message sent successfully to " << server << ":" << port << std::endl;

    freeaddrinfo(res);
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return EXIT_SUCCESS;
}

// udpsend v0.6 Copyright (C) 2024 Enrico Heine https://github.com/Flashdown/udpsend

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

#ifdef _WIN32
#define NOMINMAX // Prevent Windows headers from defining min/max macros
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

const size_t MAX_UDP_MESSAGE_SIZE = 65507;
const size_t MAX_SERVER_LENGTH = 200;     // Max length for server string (IPv4, IPv6, or domain name)
const size_t MAX_PORT_LENGTH = 5;

void printUsage(const char* progName) {
    std::cerr << std::endl << " udpsend v0.6 Copyright (C) 2024 Enrico Heine" << std::endl << std::endl;
    std::cerr << " https://github.com/Flashdown/udpsend" << std::endl << std::endl;

    std::cerr << " This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cerr << " This is free software, and you are welcome to redistribute it" << std::endl;
    std::cerr << " under the conditions of the GNU General Public License Version 3" << std::endl;

    std::cerr << "Usage: " << progName << " <server> <port> <message>" << std::endl;
    std::cerr << "  server: IPv4, IPv6 address or FQDN" << std::endl;
    std::cerr << "  port:   Port number to send the message to (1-65535)" << std::endl;
    std::cerr << "  message: Message to send via UDP (max 65507 bytes)" << std::endl;
}

bool isValidPort(const std::string& portStr) {
    std::regex portRegex("^[0-9]+$");
    if (std::regex_match(portStr, portRegex)) {
        int port = std::stoi(portStr);
        return port >= 1 && port <= 65535;
    }
    return false;
}

bool isValidServer(const std::string& server) {
    std::regex ipv4SimpleRegex("^[0-9.]+$");
    std::regex ipv6Regex("^[0-9a-fA-F:]+$");
    std::regex domainRegex("^([a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}$");

    return (std::regex_match(server, ipv4SimpleRegex) ||
            std::regex_match(server, ipv6Regex) ||
            std::regex_match(server, domainRegex));
}

bool isValidMessage(const std::string& message) {
    // Regex allows A-Z, a-z, 0-9, whitespace, special characters, and German umlauts
    std::regex messageRegex("^[A-Za-z0-9\\s!@#$%^&*()_+\\-=\\[\\]{};:'\",.<>?/|`~äöüÄÖÜß]*$");
    return std::regex_match(message, messageRegex);
}

int main(int argc, char* argv[]) {
    std::string server;
    int port;
    std::string message;

    try {
        if (argc == 4) {
            // Validate server
            server = argv[1];
            if (server.length() > MAX_SERVER_LENGTH) {
                throw std::invalid_argument("Server address is too long.");
            }
            if (!isValidServer(server)) {
                throw std::invalid_argument("Invalid server address or domain name.");
            }

            // Validate port
            if (strlen(argv[2]) > MAX_PORT_LENGTH) {
                throw std::invalid_argument("Port number is too long.");
            }
            if (!isValidPort(argv[2])) {
                throw std::invalid_argument("Invalid port number. Must be between 1 and 65535.");
            }
            port = std::stoi(argv[2]);

            // Validate message
            message = argv[3];
            if (message.length() > MAX_UDP_MESSAGE_SIZE) {
                throw std::invalid_argument("Message is too long for a single UDP packet.");
            }
            if (!isValidMessage(message)) {
                throw std::invalid_argument("Message contains invalid characters.");
            }
        } else if (argc == 3) {
            // Validate server
            server = argv[1];
            if (server.length() > MAX_SERVER_LENGTH) {
                throw std::invalid_argument("Server address is too long.");
            }
            if (!isValidServer(server)) {
                throw std::invalid_argument("Invalid server address or domain name.");
            }

            // Validate port
            if (strlen(argv[2]) > MAX_PORT_LENGTH) {
                throw std::invalid_argument("Port number is too long.");
            }
            if (!isValidPort(argv[2])) {
                throw std::invalid_argument("Invalid port number. Must be between 1 and 65535.");
            }
            port = std::stoi(argv[2]);

            // Use std::cin.read() to limit message input to MAX_UDP_MESSAGE_SIZE
            char buffer[MAX_UDP_MESSAGE_SIZE + 1];  // +1 for the null terminator
            std::cout << "Enter the message to send: ";
            std::cin.read(buffer, MAX_UDP_MESSAGE_SIZE);

            // Handle overflow if the input exceeds MAX_UDP_MESSAGE_SIZE
            if (std::cin.gcount() == MAX_UDP_MESSAGE_SIZE) {
                std::cout << "Warning: Input exceeds the maximum allowed size. Truncating input." << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            buffer[std::cin.gcount()] = '\0'; // Null-terminate the string

            message = buffer;

            // Validate message after input
            if (message.empty()) {
                throw std::invalid_argument("Message cannot be empty.");
            }
            if (!isValidMessage(message)) {
                throw std::invalid_argument("Message contains invalid characters.");
            }
        } else {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return EXIT_FAILURE;
    }
#endif

    // Create socket
#ifdef _WIN32
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }
#else
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }
#endif

    // Resolve server address
    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;       // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;   // Datagram socket

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

    // Send the message
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

    // Clean up
    freeaddrinfo(res);
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return EXIT_SUCCESS;
}

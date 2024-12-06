// udpsend v0.1 Copyright (C) 2024 Enrico Heine https://github.com/Flashdown/udpsend

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 3 as
// published by the Free Software Foundation Version 3 of the License.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " <server> <port> <message>" << std::endl;
    std::cerr << "  server: IPv4, IPv6 address or FQDN" << std::endl;
    std::cerr << "  port:   Port number to send the message to" << std::endl;
    std::cerr << "  message: Message to send via UDP" << std::endl;
    
    std::cerr << std::endl << " udpsend v0.1 Copyright (C) 2024 Enrico Heine" << std::endl << std::endl;
    std::cerr << " This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cerr << " This is free software, and you are welcome to redistribute it" << std::endl;
    std::cerr << " under the conditions of the GNU General Public License Version 3" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    std::string server = argv[1];
    int port = std::stoi(argv[2]);
    std::string message = argv[3];

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return EXIT_FAILURE;
    }
#endif

    // Create socket
    int sockfd;
#ifdef _WIN32
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }
#else
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }
#endif

    // Resolve server address
    struct addrinfo hints, * res;
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

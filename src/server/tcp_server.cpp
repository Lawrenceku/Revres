
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601   // Target Windows 7 and above
#endif
#include <winsock2.h>
#include <ws2tcpip.h>         

#include <iostream>
#include <stdexcept>
#include <string>
#include <array>

#include "tcp_server.h"

namespace revres {

static std::string wsa_error_string(int code) {
    // FormatMessage turns numeric Winsock codes (e.g. 10061) into English text.
    char* msg = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        static_cast<DWORD>(code),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&msg),
        0,
        nullptr
    );
    std::string result = msg ? msg : "(unknown error)";
    LocalFree(msg);
    // Trim trailing newline that FormatMessage appends
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();
    return result;
}


void winsock_init() {
    WSADATA wsa_data;

    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed: " + wsa_error_string(result));
    }
    std::cout << "[winsock] Initialised (version "
              << static_cast<int>(LOBYTE(wsa_data.wVersion)) << "."
              << static_cast<int>(HIBYTE(wsa_data.wVersion)) << ")\n";
}


void winsock_cleanup() {
    WSACleanup();
    std::cout << "[winsock] Cleaned up.\n";
}

void run_server(unsigned short port) {


    SOCKET listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET) {
        throw std::runtime_error("socket() failed: " + wsa_error_string(WSAGetLastError()));
    }
    std::cout << "[server] Listening socket created (fd=" << listen_fd << ")\n";

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
        closesocket(listen_fd);
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed: " + wsa_error_string(WSAGetLastError()));
    }


    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        closesocket(listen_fd);
        throw std::runtime_error("bind() failed: " + wsa_error_string(WSAGetLastError()));
    }
    std::cout << "[server] Bound to port " << port << "\n";

    if (listen(listen_fd, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listen_fd);
        throw std::runtime_error("listen() failed: " + wsa_error_string(WSAGetLastError()));
    }
    std::cout << "[server] Listening on port " << port << " ...\n\n";

    while (true) {
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);
        SOCKET client_fd = accept(listen_fd,
                                  reinterpret_cast<sockaddr*>(&client_addr),
                                  &client_addr_len);
        if (client_fd == INVALID_SOCKET) {
            std::cerr << "[server] accept() failed: " << wsa_error_string(WSAGetLastError()) << "\n";
            continue; // Try to accept the next connection rather than dying
        }

        std::cout << "──────────────────────────────────────────────\n";
        std::cout << "[client] Connected from "
                  << inet_ntoa(client_addr.sin_addr)
                  << ":" << ntohs(client_addr.sin_port) << "\n\n";

        constexpr int BUFFER_SIZE = 4096;
        std::array<char, BUFFER_SIZE> buf{};

        int bytes_received = recv(client_fd, buf.data(), BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            // Null-terminate so we can safely print as a C string.
            buf[static_cast<size_t>(bytes_received)] = '\0';
            std::cout << "── Raw bytes received (" << bytes_received << " bytes) ──\n";
            std::cout << buf.data() << "\n";
            std::cout << "────────────────────────────────────────────────\n\n";
        } else if (bytes_received == 0) {
            std::cout << "[client] Connection closed before sending data.\n";
        } else {
            std::cerr << "[server] recv() error: " << wsa_error_string(WSAGetLastError()) << "\n";
        }

        const std::string response =
            "Hello from Revres TCP server!\r\n"
            "You sent " + std::to_string(bytes_received) + " bytes.\r\n"
            "HTTP parsing is not implemented yet — stay tuned!\r\n";

        int bytes_sent = send(client_fd, response.c_str(),
                              static_cast<int>(response.size()), 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "[server] send() error: " << wsa_error_string(WSAGetLastError()) << "\n";
        } else {
            std::cout << "[server] Sent " << bytes_sent << " bytes.\n";
        }

        closesocket(client_fd);
        std::cout << "[client] Connection closed.\n\n";
    }

    closesocket(listen_fd);
}

} // namespace revres

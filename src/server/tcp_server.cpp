#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601   // Target Windows 7 and above
#endif
#include <winsock2.h>
#include <ws2tcpip.h>         

#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <atomic>
#include <thread>
#include <algorithm>

#include "tcp_server.h"
#include "../http/parser.h"
#include "../http/router.h"
#include "../utils/thread_pool.h"
#include "../utils/logger.h"

namespace revres {

static std::atomic<bool> g_server_running{true};

void stop_server() {
    g_server_running = false;
    LOG_INFO("Stop signal received. Initiating graceful shutdown...");
}

static std::string wsa_error_string(int code) {
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
    LOG_INFO("Winsock initialised.");
}

void winsock_cleanup() {
    WSACleanup();
    LOG_INFO("Winsock cleaned up.");
}

// Handle an individual client connection (runs in a worker thread)
void handle_client(SOCKET client_fd, const http::Router& router, std::string client_ip) {
    // 1. Set socket timeouts (e.g., 5 seconds) to prevent holding threads forever
    DWORD timeout = 5000;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    bool keep_alive = true;

    while (keep_alive && g_server_running) {
        constexpr int BUFFER_SIZE = 8192;
        std::array<char, BUFFER_SIZE> buf{};

        int bytes_received = recv(client_fd, buf.data(), BUFFER_SIZE - 1, 0);
        
        if (bytes_received > 0) {
            buf[static_cast<size_t>(bytes_received)] = '\0';
            
            auto req_opt = http::parse_request(buf.data());
            std::string serialized_res;
            
            if (req_opt) {
                // Determine if client wants to keep connection alive
                std::string conn_header = req_opt->get_header("connection");
                if (conn_header == "close" || req_opt->version() == "HTTP/1.0") {
                    keep_alive = false;
                }

                http::HttpResponse res;
                try {
                    res = router.route(*req_opt);
                } catch (const std::exception& e) {
                    LOG_ERR("Handler threw exception: " + std::string(e.what()));
                    res.set_status(500, "Internal Server Error");
                    res.set_content_type("text/plain");
                    res.set_body("500 Internal Server Error");
                } catch (...) {
                    LOG_ERR("Handler threw unknown exception.");
                    res.set_status(500, "Internal Server Error");
                    res.set_content_type("text/plain");
                    res.set_body("500 Internal Server Error");
                }

                // Add Connection header to response
                if (keep_alive) {
                    res.set_header("Connection", "keep-alive");
                    res.set_header("Keep-Alive", "timeout=5, max=100");
                } else {
                    res.set_header("Connection", "close");
                }

                serialized_res = res.serialize();
                LOG_INFO(req_opt->method() + " " + req_opt->path() + " -> " + std::to_string(res.serialize().substr(9, 3).empty() ? 200 : std::stoi(res.serialize().substr(9, 3)))); // Simple status log
            } else {
                http::HttpResponse bad(400, "Bad Request");
                bad.set_content_type("text/plain");
                bad.set_body("400 Bad Request");
                bad.set_header("Connection", "close");
                serialized_res = bad.serialize();
                keep_alive = false;
                LOG_WARN("Received malformed request from " + client_ip);
            }

            int bytes_sent = send(client_fd, serialized_res.c_str(), static_cast<int>(serialized_res.size()), 0);
            if (bytes_sent == SOCKET_ERROR) {
                // Could be timeout during send or client disconnected
                keep_alive = false;
            }
        } else if (bytes_received == 0) {
            // Client gracefully closed
            keep_alive = false;
        } else {
            // Error or Timeout
            int err = WSAGetLastError();
            if (err != WSAETIMEDOUT) {
                LOG_DEBUG("recv() error on " + client_ip + ": " + wsa_error_string(err));
            }
            keep_alive = false;
        }
    }

    closesocket(client_fd);
}

void run_server(unsigned short port, const http::Router& router) {
    SOCKET listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET) {
        throw std::runtime_error("socket() failed: " + wsa_error_string(WSAGetLastError()));
    }

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

    if (listen(listen_fd, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listen_fd);
        throw std::runtime_error("listen() failed: " + wsa_error_string(WSAGetLastError()));
    }

    // Initialize Thread Pool
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    utils::ThreadPool pool(num_threads);
    
    LOG_INFO("Listening on port " + std::to_string(port) + " with " + std::to_string(num_threads) + " worker threads...");

    g_server_running = true;

    while (g_server_running) {
        // Use select to wait for connections with a timeout.
        // This prevents accept() from blocking forever, allowing us to cleanly exit
        // when g_server_running becomes false.
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);

        struct timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500ms timeout

        int select_result = select(0, &read_fds, nullptr, nullptr, &timeout);
        
        if (select_result > 0 && FD_ISSET(listen_fd, &read_fds)) {
            sockaddr_in client_addr{};
            int client_addr_len = sizeof(client_addr);
            SOCKET client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
            
            if (client_fd == INVALID_SOCKET) {
                LOG_ERR("accept() failed: " + wsa_error_string(WSAGetLastError()));
                continue;
            }

            std::string client_ip = inet_ntoa(client_addr.sin_addr);
            
            // Enqueue the connection into the thread pool
            pool.enqueue([client_fd, &router, client_ip]() {
                handle_client(client_fd, router, client_ip);
            });
        }
    }

    LOG_INFO("Shutting down thread pool...");
    pool.shutdown();
    
    closesocket(listen_fd);
    LOG_INFO("Server stopped successfully.");
}

} // namespace revres

//
// main.cpp — Increment 3 entry point
//
// Responsibilities:
//   1. Parse (optional) command-line port argument
//   2. Register signal handlers for graceful shutdown (SIGINT)
//   3. Initialise Winsock
//   4. Set up the HTTP Router and Handlers
//   5. Start the TCP server loop
//   6. Clean up on exit
//

#include <iostream>
#include <stdexcept>
#include <cstdlib>    // std::atoi
#include <csignal>    // std::signal

#include "server/tcp_server.h"
#include "http/router.h"
#include "handlers/static_handler.h"
#include "utils/logger.h"

// Signal handler for graceful shutdown
void handle_sigint(int /*signal*/) {
    std::cout << "\n";
    // Notify the server loop to stop
    revres::stop_server();
}

int main(int argc, char* argv[]) {
    // Register Ctrl+C handler
    std::signal(SIGINT, handle_sigint);

    unsigned short port = 8080;
    if (argc >= 2) {
        int p = std::atoi(argv[1]);
        if (p <= 0 || p > 65535) {
            std::cerr << "Invalid port: " << argv[1] << "\n";
            return 1;
        }
        port = static_cast<unsigned short>(p);
    }

    try {
        revres::winsock_init();

        revres::http::Router router;
        revres::handlers::StaticHandler static_handler("static");

        router.set_fallback([&static_handler](const revres::http::HttpRequest& req) {
            return static_handler.handle(req);
        });

        // Test route to simulate a crash (500 error)
        router.add_route("GET", "/crash", [](const revres::http::HttpRequest&) {
            throw std::runtime_error("Simulated crash!");
            return revres::http::HttpResponse(); // unreachable
        });

        revres::run_server(port, router);
        
        revres::winsock_cleanup();
    } catch (const std::exception& e) {
        LOG_ERR(std::string("Fatal exception: ") + e.what());
        revres::winsock_cleanup();
        return 1;
    }

    return 0;
}

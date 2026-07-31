//
// main.cpp — Increment 2 entry point
//
// Responsibilities:
//   1. Parse (optional) command-line port argument
//   2. Initialise Winsock
//   3. Set up the HTTP Router and Handlers
//   4. Start the TCP server loop, passing the router
//   5. Clean up on exit
//

#include <iostream>
#include <stdexcept>
#include <cstdlib>    // std::atoi

#include "server/tcp_server.h"
#include "http/router.h"
#include "handlers/static_handler.h"

int main(int argc, char* argv[]) {
    // Allow overriding the port via: revres.exe 9090
    unsigned short port = 8080;
    if (argc >= 2) {
        int p = std::atoi(argv[1]);
        if (p <= 0 || p > 65535) {
            std::cerr << "Invalid port: " << argv[1] << "\n";
            return 1;
        }
        port = static_cast<unsigned short>(p);
    }

    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║   Revres — Increment 2 (HTTP)        ║\n";
    std::cout << "╚══════════════════════════════════════╝\n\n";

    try {
        revres::winsock_init();

        // 1. Create the router
        revres::http::Router router;

        // 2. Set up the static handler
        // Assuming we run the executable from the project root.
        // It will look for files in the "static" directory.
        revres::handlers::StaticHandler static_handler("static");

        // 3. Register the static handler as the fallback for all unmatched routes
        // We capture it by reference since it outlives the server loop
        router.set_fallback([&static_handler](const revres::http::HttpRequest& req) {
            return static_handler.handle(req);
        });

        // 4. Run the server
        revres::run_server(port, router);   // Never returns
        
        revres::winsock_cleanup();  // Called only if loop ever ends
    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL] " << e.what() << "\n";
        revres::winsock_cleanup();
        return 1;
    }

    return 0;
}

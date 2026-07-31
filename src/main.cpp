#include <iostream>
#include <stdexcept>
#include <cstdlib>    // std::atoi

#include "server/tcp_server.h"

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

    try {
        revres::winsock_init();
        revres::run_server(port);   // Never returns
        revres::winsock_cleanup();  // Called only if loop ever ends
    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL] " << e.what() << "\n";
        revres::winsock_cleanup();
        return 1;
    }

    return 0;
}

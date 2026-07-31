#pragma once
//
// server/tcp_server.h
//
// Public interface for the raw TCP server.
//
// Design principle (Increment 1):
//   We deliberately expose only what the caller (main.cpp) needs.
//   The socket lifecycle (init → create → bind → listen → accept → recv/send → close)
//   is hidden behind three clean functions.
//

#include <string>

namespace revres {

// Initialises Winsock.  Must be called once before any other socket function.
// Throws std::runtime_error on failure.
void winsock_init();

// Cleans up Winsock.  Call once at program exit.
void winsock_cleanup();

// Runs the server loop on the given port.
// Accepts one client at a time, prints every received byte to stdout,
// sends a plain-text acknowledgement, then closes the connection.
// Never returns (loops forever).  Throws std::runtime_error on fatal errors.
void run_server(unsigned short port);

} // namespace revres

#pragma once
//
// server/tcp_server.h
//
// Public interface for the raw TCP server.
//
// Design principle (Increment 2):
//   We expose the socket lifecycle but now integrate the HTTP router.
//

#include <string>
#include "../http/router.h"

namespace revres {

// Initialises Winsock.  Must be called once before any other socket function.
// Throws std::runtime_error on failure.
void winsock_init();

// Cleans up Winsock.  Call once at program exit.
void winsock_cleanup();

// Runs the server loop on the given port.
// Accepts one client at a time, passes the raw buffer to the HTTP parser,
// routes the request, and sends back the serialized HTTP response.
// Will loop until stop_server() is called. Throws std::runtime_error on fatal errors.
void run_server(unsigned short port, const http::Router& router);

// Signals the server to stop gracefully.
void stop_server();

} // namespace revres

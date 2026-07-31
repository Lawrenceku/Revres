#pragma once

#include "request.h"
#include "response.h"
#include <functional>
#include <unordered_map>
#include <string>

namespace revres {
namespace http {

using Handler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
    // Register a specific path (e.g. "/about") to a specific handler
    void add_route(const std::string& method, const std::string& path, Handler handler);

    // Set a fallback handler for paths that aren't explicitly registered
    void set_fallback(Handler handler);

    // Route an incoming request to the appropriate handler
    HttpResponse route(const HttpRequest& req) const;

private:
    // Key is "METHOD PATH", e.g., "GET /about"
    std::unordered_map<std::string, Handler> routes_;
    Handler fallback_handler_;
};

} // namespace http
} // namespace revres

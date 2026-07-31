#include "router.h"

namespace revres {
namespace http {

void Router::add_route(const std::string& method, const std::string& path, Handler handler) {
    std::string key = method + " " + path;
    routes_[key] = std::move(handler);
}

void Router::set_fallback(Handler handler) {
    fallback_handler_ = std::move(handler);
}

HttpResponse Router::route(const HttpRequest& req) const {
    std::string key = req.method() + " " + req.path();
    
    auto it = routes_.find(key);
    if (it != routes_.end()) {
        return it->second(req);
    }
    
    if (fallback_handler_) {
        return fallback_handler_(req);
    }
    
    // If no route and no fallback, return 404
    HttpResponse not_found(404, "Not Found");
    not_found.set_content_type("text/plain");
    not_found.set_body("404 Not Found - No route configured for " + req.path());
    return not_found;
}

} // namespace http
} // namespace revres

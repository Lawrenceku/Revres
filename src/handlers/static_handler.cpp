#include "static_handler.h"
#include "../utils/mime.h"
#include <fstream>
#include <sstream>

namespace revres {
namespace handlers {

StaticHandler::StaticHandler(std::string base_path)
    : base_path_(std::move(base_path)) {}

http::HttpResponse StaticHandler::handle(const http::HttpRequest& req) {
    if (req.method() != "GET") {
        http::HttpResponse res(405, "Method Not Allowed");
        res.set_content_type("text/plain");
        res.set_body("405 Method Not Allowed");
        return res;
    }

    // Default to index.html if the path is exactly "/"
    std::string request_path = req.path();
    if (request_path == "/") {
        request_path = "/index.html";
    }
    
    // Very basic security: prevent directory traversal by disallowing ".."
    if (request_path.find("..") != std::string::npos) {
        http::HttpResponse res(403, "Forbidden");
        res.set_content_type("text/plain");
        res.set_body("403 Forbidden");
        return res;
    }

    // Construct the full file path
    std::string full_path = base_path_ + request_path;
    
    // Open the file in binary mode
    std::ifstream file(full_path, std::ios::binary);
    if (!file) {
        http::HttpResponse not_found(404, "Not Found");
        not_found.set_content_type("text/plain");
        not_found.set_body("404 Not Found - Could not read " + request_path);
        return not_found;
    }

    // Read the file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    http::HttpResponse res(200, "OK");
    res.set_content_type(utils::Mime::get_mime_type(full_path));
    res.set_body(buffer.str());
    
    return res;
}

} // namespace handlers
} // namespace revres

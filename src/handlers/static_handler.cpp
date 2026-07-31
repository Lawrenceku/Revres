#include "static_handler.h"
#include <fstream>
#include <sstream>

namespace revres {
namespace handlers {

StaticHandler::StaticHandler(std::string base_path)
    : base_path_(std::move(base_path)) {}

std::string StaticHandler::get_mime_type(const std::string& path) const {
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = path.substr(dot_pos + 1);
    
    // Basic MIME types support
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "txt") return "text/plain";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "ico") return "image/x-icon";

    return "application/octet-stream";
}

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
    res.set_content_type(get_mime_type(full_path));
    res.set_body(buffer.str());
    
    return res;
}

} // namespace handlers
} // namespace revres

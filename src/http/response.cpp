#include "response.h"

namespace revres {
namespace http {

HttpResponse::HttpResponse()
    : version_("HTTP/1.1"), status_code_(200), status_message_("OK") {}

HttpResponse::HttpResponse(int status_code, std::string status_message)
    : version_("HTTP/1.1"), status_code_(status_code), status_message_(std::move(status_message)) {}

void HttpResponse::set_status(int code, const std::string& message) {
    status_code_ = code;
    status_message_ = message;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

void HttpResponse::set_body(const std::string& body) {
    body_ = body;
    // Automatically set Content-Length based on body size (in bytes)
    set_header("Content-Length", std::to_string(body_.size()));
}

void HttpResponse::set_content_type(const std::string& type) {
    set_header("Content-Type", type);
}

std::string HttpResponse::serialize() const {
    std::string raw;
    
    // 1. Status Line (e.g. "HTTP/1.1 200 OK\r\n")
    raw += version_ + " " + std::to_string(status_code_) + " " + status_message_ + "\r\n";
    
    // 2. Headers (e.g. "Content-Type: text/html\r\n")
    for (const auto& [key, value] : headers_) {
        raw += key + ": " + value + "\r\n";
    }
    
    // 3. Empty Line marking end of headers
    raw += "\r\n";
    
    // 4. Body
    raw += body_;
    
    return raw;
}

} // namespace http
} // namespace revres

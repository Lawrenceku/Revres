#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace revres {
namespace http {

//
// HttpResponse represents the HTTP response we will send back to the client.
// In HTTP/1.1, a response looks like:
// 
// HTTP/1.1 200 OK
// Content-Type: text/html
// Content-Length: 15
//
// <h1>Hello</h1>
//

class HttpResponse {
public:
    HttpResponse();
    HttpResponse(int status_code, std::string status_message);

    // Setters
    void set_status(int code, const std::string& message);
    void set_header(const std::string& key, const std::string& value);
    void set_body(const std::string& body);

    // Helper for common types
    void set_content_type(const std::string& type);

    // Convert the object into the raw text format required by HTTP/1.1
    std::string serialize() const;

private:
    std::string version_;
    int status_code_;
    std::string status_message_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
};

} // namespace http
} // namespace revres

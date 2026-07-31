#pragma once

#include <string>
#include <unordered_map>

namespace revres {
namespace http {

//
// HttpRequest represents a parsed HTTP request.
// In HTTP/1.1, a request looks like:
// 
// GET /path/to/resource HTTP/1.1
// Host: example.com
// Accept: text/html
//
// <empty line>
// [optional body]
//

class HttpRequest {
public:
    HttpRequest(std::string method, std::string path, std::string version);

    // Getters for the request line components
    const std::string& method() const { return method_; }
    const std::string& path() const { return path_; }
    const std::string& version() const { return version_; }

    // Header management
    void set_header(const std::string& key, const std::string& value);
    std::string get_header(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& headers() const { return headers_; }

private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
};

} // namespace http
} // namespace revres

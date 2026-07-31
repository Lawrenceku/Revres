#include "request.h"
#include <algorithm>
#include <cctype>

namespace revres {
namespace http {

HttpRequest::HttpRequest(std::string method, std::string path, std::string version)
    : method_(std::move(method)), path_(std::move(path)), version_(std::move(version)) {}

void HttpRequest::set_header(const std::string& key, const std::string& value) {
    // Standardize header keys to lowercase for easier lookup later
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    headers_[lower_key] = value;
}

std::string HttpRequest::get_header(const std::string& key) const {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    auto it = headers_.find(lower_key);
    if (it != headers_.end()) {
        return it->second;
    }
    return "";
}

} // namespace http
} // namespace revres

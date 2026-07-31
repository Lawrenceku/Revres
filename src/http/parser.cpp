#include "parser.h"
#include <sstream>

namespace revres {
namespace http {

std::optional<HttpRequest> parse_request(const std::string& raw_request) {
    if (raw_request.empty()) {
        return std::nullopt;
    }

    // 1. Parse the Request Line
    // Expected format: "METHOD /path HTTP/1.1\r\n"
    size_t line_end = raw_request.find("\r\n");
    if (line_end == std::string::npos) {
        return std::nullopt;
    }

    std::string request_line = raw_request.substr(0, line_end);
    std::istringstream line_stream(request_line);
    
    std::string method, path, version;
    if (!(line_stream >> method >> path >> version)) {
        return std::nullopt;
    }

    HttpRequest req(method, path, version);

    // 2. Parse Headers
    // Skip the first \r\n
    size_t pos = line_end + 2;
    
    while (pos < raw_request.size()) {
        // Find the end of the current header line
        size_t next_line = raw_request.find("\r\n", pos);
        if (next_line == std::string::npos) {
            break;
        }

        // An empty line ("\r\n\r\n") signifies the end of headers
        if (next_line == pos) {
            // End of headers found. 
            break;
        }

        std::string header_line = raw_request.substr(pos, next_line - pos);
        
        // Split header by ": "
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = header_line.substr(0, colon_pos);
            
            // Skip the colon and any leading whitespace for the value
            size_t val_start = colon_pos + 1;
            while (val_start < header_line.size() && (header_line[val_start] == ' ' || header_line[val_start] == '\t')) {
                val_start++;
            }
            std::string value = header_line.substr(val_start);
            
            req.set_header(key, value);
        }

        pos = next_line + 2;
    }

    return req;
}

} // namespace http
} // namespace revres

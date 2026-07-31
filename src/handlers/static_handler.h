#pragma once

#include "../http/request.h"
#include "../http/response.h"
#include <string>

namespace revres {
namespace handlers {

class StaticHandler {
public:
    // Initialize with the base path where static files are located
    StaticHandler(std::string base_path);

    // Handle the request and return the file content as an HTTP response
    http::HttpResponse handle(const http::HttpRequest& req);

private:
    std::string base_path_;
    
    // Helper to determine the MIME type based on file extension
    std::string get_mime_type(const std::string& path) const;
};

} // namespace handlers
} // namespace revres

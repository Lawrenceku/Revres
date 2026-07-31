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
};

} // namespace handlers
} // namespace revres

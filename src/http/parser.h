#pragma once

#include "request.h"
#include <string>
#include <optional>

namespace revres {
namespace http {

//
// Parses raw bytes received from the socket into an HttpRequest object.
// Returns std::nullopt if the request is malformed.
//
std::optional<HttpRequest> parse_request(const std::string& raw_request);

} // namespace http
} // namespace revres

#pragma once

#include <string>

namespace revres {
namespace utils {

class Mime {
public:
    // Returns the MIME type based on the file extension of the given path.
    // If the extension is unknown, returns "application/octet-stream".
    static std::string get_mime_type(const std::string& path);
};

} // namespace utils
} // namespace revres

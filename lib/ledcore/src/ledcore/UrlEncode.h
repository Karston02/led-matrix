// Percent-encoding for application/x-www-form-urlencoded bodies.
#pragma once

#include <cctype>
#include <string>

namespace ledcore {

// Encodes everything except the RFC 3986 unreserved characters.
inline std::string urlEncode(const std::string& input) {
  static const char kHex[] = "0123456789ABCDEF";
  std::string out;
  out.reserve(input.size() * 3);
  for (const char c : input) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (std::isalnum(u) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else {
      out += '%';
      out += kHex[(u >> 4) & 0x0F];
      out += kHex[u & 0x0F];
    }
  }
  return out;
}

}  // namespace ledcore

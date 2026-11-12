#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>

namespace http_json {

// GETs `url` over HTTPS and parses the JSON body into `out`, keeping only the
// fields named in `filter` (so large responses don't exhaust memory).
// Returns false on any network, size or parse error.
bool get(const String& url, const JsonDocument& filter, JsonDocument& out);

}  // namespace http_json

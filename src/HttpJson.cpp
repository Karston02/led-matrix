#include "HttpJson.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "Log.h"
#include "Tls.h"
#include "config.h"

namespace http_json {

bool get(const String& url, const JsonDocument& filter, JsonDocument& out) {
  WiFiClientSecure client;
  tls::configure(client);
  HTTPClient http;
  http.setTimeout(config::network::kApiRequestTimeoutMs);
  http.setReuse(false);
  if (!http.begin(client, url)) {
    return false;
  }

  const int code = http.GET();
  if (code != 200) {
    LOG_WARN("http", "HTTP %d", code);
    http.end();
    return false;
  }
  if (http.getSize() > static_cast<int>(config::network::kMaxJsonBytes)) {
    LOG_WARN("http", "response too large (%d bytes)", http.getSize());
    http.end();
    return false;
  }

  const String body = http.getString();  // also decodes chunked responses
  http.end();
  if (body.length() == 0 || body.length() > config::network::kMaxJsonBytes) {
    return false;
  }

  const DeserializationError error =
      deserializeJson(out, body, DeserializationOption::Filter(filter));
  if (error) {
    LOG_ERROR("http", "JSON error: %s", error.c_str());
    return false;
  }
  return true;
}

}  // namespace http_json

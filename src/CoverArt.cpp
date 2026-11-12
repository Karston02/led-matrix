#include "CoverArt.h"

#include <HTTPClient.h>
#include <TJpg_Decoder.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Log.h"
#include "Tls.h"
#include "config.h"

namespace cover_art {
namespace {

// Owns a malloc'd byte buffer; frees it when it goes out of scope.
class JpegBuffer {
 public:
  JpegBuffer() = default;
  ~JpegBuffer() { free(data_); }
  JpegBuffer(const JpegBuffer&) = delete;
  JpegBuffer& operator=(const JpegBuffer&) = delete;

  // Allocates `size` bytes, replacing any previous buffer. Returns null on failure.
  uint8_t* allocate(size_t size) {
    free(data_);
    data_ = static_cast<uint8_t*>(malloc(size));
    size_ = (data_ != nullptr) ? size : 0;
    return data_;
  }

  const uint8_t* data() const { return data_; }
  uint8_t* data() { return data_; }
  size_t size() const { return size_; }

 private:
  uint8_t* data_ = nullptr;
  size_t size_ = 0;
};

// Reads exactly `size` bytes of the response body, or gives up after a timeout.
bool readBody(HTTPClient& http, uint8_t* buffer, size_t size) {
  WiFiClient* stream = http.getStreamPtr();
  size_t received = 0;
  const uint32_t start = millis();
  while (received < size && millis() - start < config::network::kApiRequestTimeoutMs) {
    const int available = stream->available();
    if (available > 0) {
      const size_t chunk = min(static_cast<size_t>(available), size - received);
      received += stream->readBytes(buffer + received, chunk);
    } else if (!stream->connected()) {
      break;
    } else {
      delay(5);
    }
  }
  if (received != size) {
    LOG_WARN("art", "image incomplete: %u of %u bytes", static_cast<unsigned>(received),
             static_cast<unsigned>(size));
  }
  return received == size;
}

bool download(const String& url, JpegBuffer& out) {
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
    LOG_WARN("art", "image HTTP %d", code);
    http.end();
    return false;
  }

  const int size = http.getSize();
  bool ok = false;
  if (size <= 0) {
    // Chunked response (no Content-Length): let HTTPClient decode it.
    const String body = http.getString();
    if (body.length() > 0 && body.length() <= config::network::kMaxJpegBytes) {
      uint8_t* dst = out.allocate(body.length());
      if (dst != nullptr) {
        memcpy(dst, body.c_str(), body.length());
        ok = true;
      }
    }
  } else if (static_cast<size_t>(size) > config::network::kMaxJpegBytes) {
    LOG_WARN("art", "image too large: %d bytes", size);
  } else {
    uint8_t* dst = out.allocate(size);
    ok = (dst != nullptr) && readBody(http, dst, size);
  }
  http.end();
  return ok;
}

// TJpg_Decoder reports decoded blocks (up to 16x16 px) through a plain function
// pointer, so the destination has to be reachable from a static.
Frame* g_decodeTarget = nullptr;

bool onDecodedBlock(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  using config::panel::kHeight;
  using config::panel::kWidth;
  for (int row = 0; row < h; row++) {
    const int dy = y + row;
    if (dy < 0 || dy >= kHeight) continue;
    for (int col = 0; col < w; col++) {
      const int dx = x + col;
      if (dx < 0 || dx >= kWidth) continue;
      g_decodeTarget->px[dy * kWidth + dx] = bitmap[row * w + col];
    }
  }
  return true;
}

bool decode(const JpegBuffer& jpeg, Frame& out) {
  out.clear();
  g_decodeTarget = &out;
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(false);  // keep normal RGB565 byte order
  TJpgDec.setCallback(onDecodedBlock);
  const bool ok = TJpgDec.drawJpg(0, 0, jpeg.data(), static_cast<uint32_t>(jpeg.size())) == 0;
  g_decodeTarget = nullptr;
  return ok;
}

}  // namespace

bool fetch(const String& url, Frame& out) {
  JpegBuffer jpeg;
  if (!download(url, jpeg)) {
    return false;
  }
  if (!decode(jpeg, out)) {
    LOG_ERROR("art", "JPEG decode failed");
    return false;
  }
  return true;
}

}  // namespace cover_art

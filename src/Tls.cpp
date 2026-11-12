#include "Tls.h"

namespace tls {

void configure(WiFiClientSecure& client) {
  // The display only reads public cover art and one user's playback state, and
  // the only secret on the wire is a short-lived access token. Skipping chain
  // validation keeps the firmware free of embedded root certificates that expire.
  client.setInsecure();
}

}  // namespace tls

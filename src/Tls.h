#pragma once

#include <WiFiClientSecure.h>

namespace tls {

// Applies the project's TLS policy to a client. Every HTTPS request goes through
// this one function so the policy can be tightened in a single place.
//
// Current policy: certificates are NOT verified (see "Security model" in the
// README for the trade-off and how to harden it).
void configure(WiFiClientSecure& client);

}  // namespace tls

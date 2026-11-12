// Minimal tagged logging over the serial port.
//
//   LOG_INFO("wifi", "connected, rssi %d", rssi);
//
// Never log tokens, passwords or full request/response bodies.
#pragma once

#include <Arduino.h>

#define LOG_INFO(tag, fmt, ...) Serial.printf("[I][" tag "] " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(tag, fmt, ...) Serial.printf("[W][" tag "] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(tag, fmt, ...) Serial.printf("[E][" tag "] " fmt "\n", ##__VA_ARGS__)

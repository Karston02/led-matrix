#pragma once

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Frame.h"

// Owns the HUB75 panel: initialisation, brightness, text, and gamma-corrected
// crossfades between frames.
class Display {
 public:
  Display() = default;
  ~Display();
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;

  // Brings up the DMA driver (double-buffered if possible). False on failure.
  bool begin();

  void setBrightness(uint8_t level);
  void clear();
  void showMessage(const char* line1, const char* line2 = nullptr);

  // Drawing API for screens: beginFrame(), draw, present().
  void beginFrame();
  void present();
  void drawCenteredText(const char* text, int y, int size, uint8_t r, uint8_t g, uint8_t b);
  MatrixPanel_I2S_DMA& gfx() { return *panel_; }

  // Draws a blend of two frames (mix 0 = `from`, 255 = `to`) and presents it.
  void drawBlend(const Frame& from, const Frame& to, uint8_t mix);
  void crossfade(const Frame& from, const Frame& to);

 private:
  MatrixPanel_I2S_DMA* panel_ = nullptr;
  uint8_t gamma_[256] = {};
};

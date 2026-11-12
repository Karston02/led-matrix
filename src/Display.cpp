#include "Display.h"

#include <ledcore/Color.h>

#include "Log.h"
#include "config.h"

namespace {

constexpr int kCharWidth = 6;  // Adafruit GFX default font, per text-size step

}  // namespace

Display::~Display() {
  delete panel_;
}

bool Display::begin() {
  using namespace config;
  ledcore::buildGammaLut(gamma_, display::kGamma);

  HUB75_I2S_CFG::i2s_pins wiring = {pins::kR1, pins::kG1,  pins::kB1, pins::kR2, pins::kG2,
                                    pins::kB2, pins::kA,   pins::kB,  pins::kC,  pins::kD,
                                    pins::kE,  pins::kLat, pins::kOe, pins::kClk};

  // Try double buffering first (flicker-free updates); fall back to single.
  for (int attempt = 0; attempt < 2; attempt++) {
    HUB75_I2S_CFG cfg(panel::kWidth, panel::kHeight, 1, wiring);
    cfg.double_buff = (attempt == 0);
    // If the panel stays black or shows garbage, try:
    // cfg.driver = HUB75_I2S_CFG::FM6126A;
    panel_ = new MatrixPanel_I2S_DMA(cfg);
    if (panel_->begin()) {
      LOG_INFO("display", "panel ready (double buffer: %s)", attempt == 0 ? "yes" : "no");
      panel_->clearScreen();
      return true;
    }
    delete panel_;
    panel_ = nullptr;
  }
  return false;
}

void Display::setBrightness(uint8_t level) {
  panel_->setBrightness8(level);
}

void Display::clear() {
  panel_->fillScreen(0);
  panel_->flipDMABuffer();
}

void Display::showMessage(const char* line1, const char* line2) {
  panel_->fillScreen(0);
  panel_->setTextSize(1);
  panel_->setTextWrap(false);
  panel_->setTextColor(panel_->color565(200, 200, 200));
  panel_->setCursor(2, 24);
  panel_->print(line1);
  if (line2 != nullptr) {
    panel_->setCursor(2, 36);
    panel_->print(line2);
  }
  panel_->flipDMABuffer();
}

void Display::beginFrame() {
  panel_->fillScreen(0);
  panel_->setTextWrap(false);
}

void Display::present() {
  panel_->flipDMABuffer();
}

void Display::drawCenteredText(const char* text, int y, int size, uint8_t r, uint8_t g,
                               uint8_t b) {
  const int width = static_cast<int>(strlen(text)) * kCharWidth * size;
  panel_->setTextSize(size);
  panel_->setTextColor(panel_->color565(r, g, b));
  panel_->setCursor(max(0, (config::panel::kWidth - width) / 2), y);
  panel_->print(text);
}

void Display::drawBlend(const Frame& from, const Frame& to, uint8_t mix) {
  using config::panel::kHeight;
  using config::panel::kWidth;
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      const int i = y * kWidth + x;
      const ledcore::Rgb c =
          ledcore::blend(ledcore::unpack565(from.px[i]), ledcore::unpack565(to.px[i]), mix);
      panel_->drawPixelRGB888(x, y, gamma_[c.r], gamma_[c.g], gamma_[c.b]);
    }
  }
  panel_->flipDMABuffer();
}

void Display::crossfade(const Frame& from, const Frame& to) {
  using config::display::kFadeStepMs;
  using config::display::kFadeSteps;
  for (int step = 1; step <= kFadeSteps; step++) {
    drawBlend(from, to, static_cast<uint8_t>(step * 255 / kFadeSteps));
    delay(kFadeStepMs);
  }
}

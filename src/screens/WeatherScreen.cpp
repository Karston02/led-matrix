#include "WeatherScreen.h"

#include <stdio.h>

#include "config.h"

namespace {

using ledcore::WeatherIcon;
using Gfx = MatrixPanel_I2S_DMA;

constexpr int kIconCenterX = 32;
constexpr int kIconCenterY = 17;

void drawCloud(Gfx& g, int cx, int cy, uint16_t color) {
  g.fillCircle(cx - 7, cy + 2, 5, color);
  g.fillCircle(cx, cy - 2, 7, color);
  g.fillCircle(cx + 7, cy + 2, 5, color);
  g.fillRect(cx - 7, cy + 2, 15, 5, color);
}

void drawSun(Gfx& g, int cx, int cy, int radius, uint16_t color) {
  g.fillCircle(cx, cy, radius, color);
  const int inner = radius + 2;
  const int outer = radius + 5;
  // Four straight rays and four diagonal ones.
  g.drawLine(cx + inner, cy, cx + outer, cy, color);
  g.drawLine(cx - inner, cy, cx - outer, cy, color);
  g.drawLine(cx, cy + inner, cx, cy + outer, color);
  g.drawLine(cx, cy - inner, cx, cy - outer, color);
  const int d1 = (inner * 7) / 10;
  const int d2 = (outer * 7) / 10;
  for (int sx = -1; sx <= 1; sx += 2) {
    for (int sy = -1; sy <= 1; sy += 2) {
      g.drawLine(cx + sx * d1, cy + sy * d1, cx + sx * d2, cy + sy * d2, color);
    }
  }
}

void drawIcon(Gfx& g, WeatherIcon icon, int cx, int cy) {
  const uint16_t yellow = g.color565(255, 190, 30);
  const uint16_t moon = g.color565(200, 200, 170);
  const uint16_t lightGray = g.color565(190, 190, 200);
  const uint16_t darkGray = g.color565(110, 110, 125);
  const uint16_t blue = g.color565(60, 140, 255);
  const uint16_t white = g.color565(240, 240, 255);

  switch (icon) {
    case WeatherIcon::ClearDay:
      drawSun(g, cx, cy, 6, yellow);
      break;
    case WeatherIcon::ClearNight:
      g.fillCircle(cx, cy, 9, moon);
      g.fillCircle(cx + 5, cy - 4, 8, 0);  // bite out of the disc
      break;
    case WeatherIcon::PartlyCloudy:
      g.fillCircle(cx + 6, cy - 6, 6, yellow);
      drawCloud(g, cx - 2, cy + 3, lightGray);
      break;
    case WeatherIcon::Cloudy:
      drawCloud(g, cx, cy, lightGray);
      break;
    case WeatherIcon::Fog:
      for (int i = 0; i < 4; i++) {
        g.drawFastHLine(cx - 12 + (i % 2) * 3, cy - 7 + i * 5, 20, darkGray);
      }
      break;
    case WeatherIcon::Rain:
      drawCloud(g, cx, cy - 5, darkGray);
      for (int dx = -6; dx <= 6; dx += 6) {
        g.drawLine(cx + dx, cy + 6, cx + dx - 2, cy + 11, blue);
      }
      break;
    case WeatherIcon::Snow:
      drawCloud(g, cx, cy - 5, darkGray);
      for (int dx = -6; dx <= 6; dx += 6) {
        g.fillRect(cx + dx, cy + 6 + (dx == 0 ? 3 : 0), 2, 2, white);
      }
      break;
    case WeatherIcon::Thunder:
      drawCloud(g, cx, cy - 5, darkGray);
      g.drawLine(cx + 1, cy + 3, cx - 3, cy + 8, yellow);
      g.drawLine(cx - 3, cy + 8, cx + 2, cy + 8, yellow);
      g.drawLine(cx + 2, cy + 8, cx - 2, cy + 13, yellow);
      break;
  }
}

}  // namespace

uint32_t WeatherScreen::durationMs() const {
  return config::carousel::kWeatherMs;
}

void WeatherScreen::draw(Display& display, uint32_t) {
  const WeatherData& weather = weather_.data();

  display.beginFrame();
  drawIcon(display.gfx(), weather.icon, kIconCenterX, kIconCenterY);

  // Temperature at size 2 (12 px per digit) with a hand-drawn degree mark,
  // because the default font has no reliable degree glyph.
  char temp[8];
  snprintf(temp, sizeof(temp), "%d", weather.tempNow);
  const int textWidth = static_cast<int>(strlen(temp)) * 12;
  const int startX = (config::panel::kWidth - (textWidth + 7)) / 2;
  Gfx& g = display.gfx();
  g.setTextSize(2);
  g.setTextColor(g.color565(230, 230, 230));
  g.setCursor(startX, 36);
  g.print(temp);
  g.drawCircle(startX + textWidth + 3, 38, 2, g.color565(230, 230, 230));

  char range[16];
  snprintf(range, sizeof(range), "H%d L%d", weather.tempHigh, weather.tempLow);
  display.drawCenteredText(range, 55, 1, 120, 120, 120);
  display.present();
}

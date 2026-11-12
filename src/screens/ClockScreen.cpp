#include "ClockScreen.h"

#include "config.h"

uint32_t ClockScreen::durationMs() const {
  return config::carousel::kClockMs;
}

void ClockScreen::draw(Display& display, uint32_t) {
  tm now;
  if (!time_.localTime(now)) {
    display.showMessage("Syncing", "time...");
    return;
  }

  char timeText[12];
  char dateText[16];
  strftime(timeText, sizeof(timeText), "%l:%M", &now);  // e.g. " 9:41"
  strftime(dateText, sizeof(dateText), "%a %b %d", &now);
  const char* shownTime = timeText;
  while (*shownTime == ' ') shownTime++;  // drop the leading space %l leaves for 1-9

  display.beginFrame();
  display.drawCenteredText(shownTime, 16, 2, 150, 100, 20);
  display.drawCenteredText(dateText, 42, 1, 70, 70, 70);
  display.present();
}

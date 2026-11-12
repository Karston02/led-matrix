// Unit tests for the hardware-independent logic. Run with: pio test -e native
#include <unity.h>

#include <string>

#include <ledcore/Color.h>
#include <ledcore/PlaybackPolicy.h>
#include <ledcore/Rotation.h>
#include <ledcore/Schedule.h>
#include <ledcore/UrlEncode.h>
#include <ledcore/Weather.h>

using namespace ledcore;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------- Color

void test_unpack565_extremes() {
  const Rgb black = unpack565(0x0000);
  TEST_ASSERT_EQUAL_UINT8(0, black.r);
  TEST_ASSERT_EQUAL_UINT8(0, black.g);
  TEST_ASSERT_EQUAL_UINT8(0, black.b);

  const Rgb white = unpack565(0xFFFF);
  TEST_ASSERT_EQUAL_UINT8(255, white.r);
  TEST_ASSERT_EQUAL_UINT8(255, white.g);
  TEST_ASSERT_EQUAL_UINT8(255, white.b);
}

void test_unpack565_primaries() {
  const Rgb red = unpack565(0xF800);
  TEST_ASSERT_EQUAL_UINT8(255, red.r);
  TEST_ASSERT_EQUAL_UINT8(0, red.g);
  TEST_ASSERT_EQUAL_UINT8(0, red.b);

  const Rgb green = unpack565(0x07E0);
  TEST_ASSERT_EQUAL_UINT8(0, green.r);
  TEST_ASSERT_EQUAL_UINT8(255, green.g);
  TEST_ASSERT_EQUAL_UINT8(0, green.b);

  const Rgb blue = unpack565(0x001F);
  TEST_ASSERT_EQUAL_UINT8(0, blue.r);
  TEST_ASSERT_EQUAL_UINT8(0, blue.g);
  TEST_ASSERT_EQUAL_UINT8(255, blue.b);
}

void test_blend_endpoints() {
  TEST_ASSERT_EQUAL_UINT8(40, blendChannel(40, 200, 0));
  TEST_ASSERT_EQUAL_UINT8(200, blendChannel(40, 200, 255));
}

void test_blend_midpoint() {
  const uint8_t mid = blendChannel(0, 200, 128);
  TEST_ASSERT_TRUE(mid >= 99 && mid <= 101);
}

void test_blend_rgb() {
  const Rgb out = blend(Rgb{0, 0, 0}, Rgb{255, 100, 50}, 255);
  TEST_ASSERT_EQUAL_UINT8(255, out.r);
  TEST_ASSERT_EQUAL_UINT8(100, out.g);
  TEST_ASSERT_EQUAL_UINT8(50, out.b);
}

void test_gamma_lut_shape() {
  uint8_t lut[256];
  buildGammaLut(lut, 2.2f);
  TEST_ASSERT_EQUAL_UINT8(0, lut[0]);
  TEST_ASSERT_EQUAL_UINT8(255, lut[255]);
  TEST_ASSERT_TRUE(lut[128] < 128);  // mid-tones are darkened
  for (int i = 1; i < 256; i++) {
    TEST_ASSERT_TRUE(lut[i] >= lut[i - 1]);  // monotonic
  }
}

void test_gamma_of_one_is_identity() {
  uint8_t lut[256];
  buildGammaLut(lut, 1.0f);
  for (int i = 0; i < 256; i++) {
    TEST_ASSERT_EQUAL_UINT8(i, lut[i]);
  }
}

// ---------------------------------------------------------------- Schedule

void test_night_window_wrapping_midnight() {
  TEST_ASSERT_TRUE(isNightHour(23, 23, 7));
  TEST_ASSERT_TRUE(isNightHour(0, 23, 7));
  TEST_ASSERT_TRUE(isNightHour(6, 23, 7));
  TEST_ASSERT_FALSE(isNightHour(7, 23, 7));
  TEST_ASSERT_FALSE(isNightHour(12, 23, 7));
  TEST_ASSERT_FALSE(isNightHour(22, 23, 7));
}

void test_night_window_same_day() {
  TEST_ASSERT_FALSE(isNightHour(0, 1, 5));
  TEST_ASSERT_TRUE(isNightHour(1, 1, 5));
  TEST_ASSERT_TRUE(isNightHour(4, 1, 5));
  TEST_ASSERT_FALSE(isNightHour(5, 1, 5));
}

// ---------------------------------------------------------------- UrlEncode

void test_url_encode_unreserved_passthrough() {
  TEST_ASSERT_EQUAL_STRING("AZaz09-_.~", urlEncode("AZaz09-_.~").c_str());
}

void test_url_encode_reserved() {
  TEST_ASSERT_EQUAL_STRING("a%20b", urlEncode("a b").c_str());
  TEST_ASSERT_EQUAL_STRING("a%2Bb%2Fc%3D%26", urlEncode("a+b/c=&").c_str());
}

void test_url_encode_empty_and_high_bytes() {
  TEST_ASSERT_EQUAL_STRING("", urlEncode("").c_str());
  TEST_ASSERT_EQUAL_STRING("%C3%A9", urlEncode("\xC3\xA9").c_str());
}

// ---------------------------------------------------------------- PlaybackPolicy

static const PollTiming kTiming = {3000, 6000, 10000, 60000};

static PollContext ctx(NowStatus status, bool idle, bool hasImage, bool changed,
                       uint32_t sincePlaying) {
  return PollContext{status, idle, hasImage, changed, sincePlaying};
}

void test_playing_new_cover_shows_art() {
  const PollDecision d = decidePoll(ctx(NowStatus::Playing, true, true, true, 0), kTiming);
  TEST_ASSERT_TRUE(d.showArt);
  TEST_ASSERT_FALSE(d.enterIdle);
  TEST_ASSERT_EQUAL_UINT32(3000, d.nextPollMs);
}

void test_playing_same_cover_does_nothing() {
  const PollDecision d = decidePoll(ctx(NowStatus::Playing, false, true, false, 0), kTiming);
  TEST_ASSERT_FALSE(d.showArt);
  TEST_ASSERT_FALSE(d.enterIdle);
}

void test_playing_without_image_does_not_show_art() {
  const PollDecision d = decidePoll(ctx(NowStatus::Playing, false, false, true, 0), kTiming);
  TEST_ASSERT_FALSE(d.showArt);
}

void test_paused_within_grace_keeps_cover() {
  const PollDecision d = decidePoll(ctx(NowStatus::Paused, false, true, false, 59000), kTiming);
  TEST_ASSERT_FALSE(d.enterIdle);
  TEST_ASSERT_EQUAL_UINT32(3000, d.nextPollMs);
}

void test_paused_after_grace_enters_idle() {
  const PollDecision d = decidePoll(ctx(NowStatus::Paused, false, true, false, 60001), kTiming);
  TEST_ASSERT_TRUE(d.enterIdle);
}

void test_paused_while_idle_does_not_show_art() {
  const PollDecision d = decidePoll(ctx(NowStatus::Paused, true, true, true, 999999), kTiming);
  TEST_ASSERT_FALSE(d.showArt);
  TEST_ASSERT_FALSE(d.enterIdle);
}

void test_nothing_playing_while_idle_polls_slowly() {
  const PollDecision d = decidePoll(ctx(NowStatus::Nothing, true, false, false, 0), kTiming);
  TEST_ASSERT_EQUAL_UINT32(6000, d.nextPollMs);
  TEST_ASSERT_FALSE(d.enterIdle);
}

void test_nothing_playing_after_grace_enters_idle() {
  const PollDecision d = decidePoll(ctx(NowStatus::Nothing, false, false, false, 61000), kTiming);
  TEST_ASSERT_TRUE(d.enterIdle);
  TEST_ASSERT_EQUAL_UINT32(3000, d.nextPollMs);
}

void test_error_backs_off_and_changes_nothing() {
  const PollDecision d = decidePoll(ctx(NowStatus::Error, false, true, true, 999999), kTiming);
  TEST_ASSERT_EQUAL_UINT32(10000, d.nextPollMs);
  TEST_ASSERT_FALSE(d.showArt);
  TEST_ASSERT_FALSE(d.enterIdle);
}

// ---------------------------------------------------------------- Weather

void test_weather_icons() {
  TEST_ASSERT_TRUE(iconForWmoCode(0, true) == WeatherIcon::ClearDay);
  TEST_ASSERT_TRUE(iconForWmoCode(0, false) == WeatherIcon::ClearNight);
  TEST_ASSERT_TRUE(iconForWmoCode(2, true) == WeatherIcon::PartlyCloudy);
  TEST_ASSERT_TRUE(iconForWmoCode(2, false) == WeatherIcon::Cloudy);
  TEST_ASSERT_TRUE(iconForWmoCode(3, true) == WeatherIcon::Cloudy);
  TEST_ASSERT_TRUE(iconForWmoCode(45, true) == WeatherIcon::Fog);
  TEST_ASSERT_TRUE(iconForWmoCode(53, true) == WeatherIcon::Rain);
  TEST_ASSERT_TRUE(iconForWmoCode(81, true) == WeatherIcon::Rain);
  TEST_ASSERT_TRUE(iconForWmoCode(73, true) == WeatherIcon::Snow);
  TEST_ASSERT_TRUE(iconForWmoCode(86, true) == WeatherIcon::Snow);
  TEST_ASSERT_TRUE(iconForWmoCode(95, true) == WeatherIcon::Thunder);
  TEST_ASSERT_TRUE(iconForWmoCode(1234, true) == WeatherIcon::Cloudy);
}

// ---------------------------------------------------------------- Rotation

void test_next_available_skips_unavailable() {
  const bool available[] = {true, false, true};
  TEST_ASSERT_EQUAL_UINT32(2, nextAvailable(available, 3, 0));
  TEST_ASSERT_EQUAL_UINT32(0, nextAvailable(available, 3, 2));  // wraps
}

void test_next_available_stays_when_alone() {
  const bool available[] = {true, false, false};
  TEST_ASSERT_EQUAL_UINT32(0, nextAvailable(available, 3, 0));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_unpack565_extremes);
  RUN_TEST(test_unpack565_primaries);
  RUN_TEST(test_blend_endpoints);
  RUN_TEST(test_blend_midpoint);
  RUN_TEST(test_blend_rgb);
  RUN_TEST(test_gamma_lut_shape);
  RUN_TEST(test_gamma_of_one_is_identity);
  RUN_TEST(test_night_window_wrapping_midnight);
  RUN_TEST(test_night_window_same_day);
  RUN_TEST(test_url_encode_unreserved_passthrough);
  RUN_TEST(test_url_encode_reserved);
  RUN_TEST(test_url_encode_empty_and_high_bytes);
  RUN_TEST(test_playing_new_cover_shows_art);
  RUN_TEST(test_playing_same_cover_does_nothing);
  RUN_TEST(test_playing_without_image_does_not_show_art);
  RUN_TEST(test_paused_within_grace_keeps_cover);
  RUN_TEST(test_paused_after_grace_enters_idle);
  RUN_TEST(test_paused_while_idle_does_not_show_art);
  RUN_TEST(test_nothing_playing_while_idle_polls_slowly);
  RUN_TEST(test_nothing_playing_after_grace_enters_idle);
  RUN_TEST(test_error_backs_off_and_changes_nothing);
  RUN_TEST(test_weather_icons);
  RUN_TEST(test_next_available_skips_unavailable);
  RUN_TEST(test_next_available_stays_when_alone);
  return UNITY_END();
}

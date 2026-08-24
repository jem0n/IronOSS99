#include "ui_drawing.hpp"

#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI

void ui_draw_temperature_change(void) {
  const bool reverse = getSettingValue(SettingsOptions::ReverseButtonTempChangeEnabled);
  const bool left    = OLED::getRotation();
  // Button next to each edge; the sign matches which button raises / lowers the temperature
  OLED::setCursor(2, 8);
  OLED::print((left != reverse) ? LargeSymbolMinus : LargeSymbolPlus, FontStyle::LARGE);
  OLED::setCursor(16, 0);
  OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::HUGE);
  OLED::printSymbolDeg(FontStyle::HUGE);
  OLED::setCursor(114, 8);
  OLED::print((left != reverse) ? LargeSymbolPlus : LargeSymbolMinus, FontStyle::LARGE);
}

#else /* scaled 96x16 layout */

void ui_draw_temperature_change(void) {

  OLED::setCursor(8, 8);
  if (OLED::getRotation()) {
    OLED::print(getSettingValue(SettingsOptions::ReverseButtonTempChangeEnabled) ? LargeSymbolPlus : LargeSymbolMinus, FontStyle::LARGE);
  } else {
    OLED::print(getSettingValue(SettingsOptions::ReverseButtonTempChangeEnabled) ? LargeSymbolMinus : LargeSymbolPlus, FontStyle::LARGE);
  }

  OLED::print(LargeSymbolSpace, FontStyle::LARGE);
  OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::LARGE);
  OLED::printSymbolDeg(FontStyle::EXTRAS);
  OLED::print(LargeSymbolSpace, FontStyle::LARGE);
  if (OLED::getRotation()) {
    OLED::print(getSettingValue(SettingsOptions::ReverseButtonTempChangeEnabled) ? LargeSymbolMinus : LargeSymbolPlus, FontStyle::LARGE);
  } else {
    OLED::print(getSettingValue(SettingsOptions::ReverseButtonTempChangeEnabled) ? LargeSymbolPlus : LargeSymbolMinus, FontStyle::LARGE);
  }
}
#endif /* OLED_128x32_HIRES_UI */
#endif
#include "OperatingModes.h"
#include "ui_drawing.hpp"

#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI

void ui_draw_soldering_detailed_sleep(TemperatureType_t tipTemp) {
  OLED::clearScreen();
  OLED::setCursor(0, 0);
  OLED::print(translatedString(Tr->SleepingAdvancedString), FontStyle::SMALL);
  OLED::setCursor(0, 8);
  OLED::print(translatedString(Tr->SleepingTipAdvancedString), FontStyle::SMALL);
  OLED::printNumber(tipTemp, 3, FontStyle::SMALL);
  OLED::printSymbolDeg(FontStyle::SMALL);

  OLED::setCursor(0, 16);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::SMALL, 2);

  OLED::setCursor(0, 24);
  OLED::printNumber(getSettingValue(SettingsOptions::SleepTemp), 3, FontStyle::SMALL);
  OLED::printSymbolDeg(FontStyle::SMALL);

  OLED::refresh();
}

void ui_draw_soldering_basic_sleep(TemperatureType_t tipTemp) {
  OLED::clearScreen();
  // [Zzz 36px][HUGE temp 72px][deg symbol 12px]
  OLED::setCursor(0, 0);
  OLED::print(LargeSymbolSleep, FontStyle::LARGE, 3);
  OLED::setCursor(36, 0);
  OLED::printNumber(tipTemp, 3, FontStyle::HUGE);
  OLED::setCursor(108, 8);
  OLED::printSymbolDeg(FontStyle::EXTRAS);

  OLED::refresh();
}

#else  /* scaled 96x16 layout */

void ui_draw_soldering_detailed_sleep(TemperatureType_t tipTemp) {

  OLED::clearScreen();
  OLED::setCursor(0, 0);
  OLED::print(translatedString(Tr->SleepingAdvancedString), FontStyle::SMALL);
  OLED::setCursor(0, 8);
  OLED::print(translatedString(Tr->SleepingTipAdvancedString), FontStyle::SMALL);
  OLED::printNumber(tipTemp, 3, FontStyle::SMALL);
  if (getSettingValue(SettingsOptions::TemperatureInF)) {
    OLED::print(SmallSymbolDegF, FontStyle::SMALL);
  } else {
    OLED::print(SmallSymbolDegC, FontStyle::SMALL);
  }

  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);

  OLED::refresh();
}

void ui_draw_soldering_basic_sleep(TemperatureType_t tipTemp) {

  OLED::clearScreen();
  OLED::setCursor(0, 0);

  OLED::print(LargeSymbolSleep, FontStyle::LARGE);
  OLED::printNumber(tipTemp, 3, FontStyle::LARGE);
  OLED::printSymbolDeg(FontStyle::EXTRAS);

  OLED::refresh();
}
#endif /* OLED_128x32_HIRES_UI */
#endif
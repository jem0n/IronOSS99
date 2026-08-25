#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI

// Full height layout: [HUGE temp + deg symbol 96px][5 char info column: watts / volts / source+countdown / set temp]
void ui_draw_soldering_power_status(bool boost_mode_on) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t tempX      = leftHanded ? 32 : 0;
  const int16_t infoX      = leftHanded ? 0 : 98;

  OLED::setCursor(tempX, 0);
  ui_draw_tip_temperature(true, FontStyle::HUGE);

  // Line 0: wattage
  OLED::setCursor(infoX, 0);
  {
    uint32_t x10Watt = x10WattHistory.average();
    if (x10Watt > 999) {
      // If we exceed 99.9W we drop the decimal place to keep it all fitting
      OLED::printNumber(x10Watt / 10, 3, FontStyle::SMALL);
    } else {
      OLED::printNumber(x10Watt / 10, 2, FontStyle::SMALL);
      OLED::print(SmallSymbolDot, FontStyle::SMALL);
      OLED::printNumber(x10Watt % 10, 1, FontStyle::SMALL);
    }
    OLED::print(SmallSymbolWatts, FontStyle::SMALL);
  }
  // Line 1: input voltage
  OLED::setCursor(infoX, 8);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);

  // Line 2: power source + countdown to sleep
  OLED::setCursor(infoX, 16);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::SMALL, 2);
#ifndef NO_SLEEP_MODE
  if (!boost_mode_on && getSettingValue(SettingsOptions::Sensitivity) && getSettingValue(SettingsOptions::SleepTime)) {
    printCountdownUntilSleep(getSleepTimeout());
  }
#endif

  // Line 3: set point, or boost indicator
  OLED::setCursor(infoX, 24);
  if (boost_mode_on) {
    OLED::print(SmallSymbolPlus, FontStyle::SMALL);
    OLED::printNumber(getSettingValue(SettingsOptions::BoostTemp), 3, FontStyle::SMALL);
  } else {
    OLED::print(SmallSymbolSpace, FontStyle::SMALL);
    OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::SMALL);
  }
  OLED::printSymbolDeg(FontStyle::SMALL);
}

#else /* scaled 96x16 layout */


void ui_draw_soldering_power_status(bool boost_mode_on) {
  if (OLED::getRotation()) {
    OLED::setCursor(50, 0);
  } else {
    OLED::setCursor(-1, 0);
  }

  ui_draw_tip_temperature(true, FontStyle::LARGE);

  if (boost_mode_on) { // Boost mode is on
    if (OLED::getRotation()) {
      OLED::setCursor(34, 0);
    } else {
      OLED::setCursor(50, 0);
    }
    OLED::print(LargeSymbolPlus, FontStyle::LARGE);
  } else {
#ifndef NO_SLEEP_MODE
    if (getSettingValue(SettingsOptions::Sensitivity) && getSettingValue(SettingsOptions::SleepTime)) {
      if (OLED::getRotation()) {
        OLED::setCursor(32, 0);
      } else {
        OLED::setCursor(47, 0);
      }
      printCountdownUntilSleep(getSleepTimeout());
    }
#endif
    if (OLED::getRotation()) {
      OLED::setCursor(32, 8);
    } else {
      OLED::setCursor(47, 8);
    }
    OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::SMALL, 2);
  }

  if (OLED::getRotation()) {
    OLED::setCursor(0, 0);
  } else {
    OLED::setCursor(67, 0);
  }
  // Print wattage
  {
    uint32_t x10Watt = x10WattHistory.average();
    if (x10Watt > 999) {
      // If we exceed 99.9W we drop the decimal place to keep it all fitting
      OLED::print(SmallSymbolSpace, FontStyle::SMALL);
      OLED::printNumber(x10WattHistory.average() / 10, 3, FontStyle::SMALL);
    } else {
      OLED::printNumber(x10WattHistory.average() / 10, 2, FontStyle::SMALL);
      OLED::print(SmallSymbolDot, FontStyle::SMALL);
      OLED::printNumber(x10WattHistory.average() % 10, 1, FontStyle::SMALL);
    }
    OLED::print(SmallSymbolWatts, FontStyle::SMALL);
  }

  if (OLED::getRotation()) {
    OLED::setCursor(0, 8);
  } else {
    OLED::setCursor(67, 8);
  }
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);
}
#endif /* OLED_128x32_HIRES_UI */
#endif
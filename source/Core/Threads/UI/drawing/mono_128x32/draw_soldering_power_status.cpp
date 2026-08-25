#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI

/*
 * Data dense full height layout (right handed; the two blocks swap for left handed):
 *
 *   x 0..59                       x 60..127 (11 small chars)
 *   +----------------------------+-----------------------------+
 *   | 350°  (LARGE 16px)         | 45.3W 20.1V                 |  y 0
 *   |                            | PD 41° 2.3A  src/handle/A   |  y 8
 *   | [######    ] power bar     | 5.5Ω 11kHz   tip R / chop f |  y 16
 *   |  320° 12s   set / sleep    | Max130W 63%  limit / duty   |  y 24
 *   +----------------------------+-----------------------------+
 */
static void printX10Watts(uint32_t x10Watt) {
  if (x10Watt > 999) {
    // >= 100 W: drop the decimal so it still fits in 4 chars
    OLED::printNumber(x10Watt / 10, 3, FontStyle::SMALL);
  } else {
    OLED::printNumber(x10Watt / 10, 2, FontStyle::SMALL);
    OLED::print(SmallSymbolDot, FontStyle::SMALL);
    OLED::printNumber(x10Watt % 10, 1, FontStyle::SMALL);
  }
  OLED::print(SmallSymbolWatts, FontStyle::SMALL);
}

static void drawPowerBar(int16_t x, uint32_t x10Watt, int32_t x10Limit) {
  // 48 px wide bar, 6 px tall, filled proportional to the power vs the currently effective limit
  const uint8_t width = 46;
  uint32_t      len   = 0;
  if (x10Limit > 0) {
    len = (x10Watt * width) / (uint32_t)x10Limit;
    if (len > width) {
      len = width;
    }
  }
  // Frame: top & bottom rails
  OLED::drawFilledRect(x, 16, x + width + 1, 16, false);
  OLED::drawFilledRect(x, 22, x + width + 1, 22, false);
  OLED::drawFilledRect(x, 16, x, 22, false);
  OLED::drawFilledRect(x + width + 1, 16, x + width + 1, 22, false);
  if (len) {
    OLED::drawFilledRect(x + 1, 18, x + len, 20, false);
  }
}

void ui_draw_soldering_power_status(bool boost_mode_on) {
  const bool     leftHanded = OLED::getRotation();
  const int16_t  mainX      = leftHanded ? 68 : 0;
  const int16_t  infoX      = leftHanded ? 0 : 60;
  const uint32_t x10Watt    = x10WattHistory.average();
  const uint32_t voltX10    = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  const int32_t  x10Limit   = getX10WattageLimits();

  // Main block: tip temperature, power bar, set point + sleep countdown
  OLED::setCursor(mainX, 0);
  ui_draw_tip_temperature(true, FontStyle::LARGE);

  drawPowerBar(mainX, x10Watt, x10Limit);

  OLED::setCursor(mainX, 24);
  if (boost_mode_on) {
    OLED::print(SmallSymbolPlus, FontStyle::SMALL);
    OLED::printNumber(getSettingValue(SettingsOptions::BoostTemp), 3, FontStyle::SMALL);
  } else {
    OLED::print(SmallSymbolSpace, FontStyle::SMALL);
    OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::SMALL);
  }
  OLED::printSymbolDeg(FontStyle::SMALL);
#ifndef NO_SLEEP_MODE
  if (!boost_mode_on && getSettingValue(SettingsOptions::Sensitivity) && getSettingValue(SettingsOptions::SleepTime)) {
    OLED::setCursor(mainX + 32, 24);
    printCountdownUntilSleep(getSleepTimeout());
  }
#endif

  // Info block, line 0: wattage + input voltage
  OLED::setCursor(infoX, 0);
  printX10Watts(x10Watt);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);

  // Line 1: power source, handle temperature, tip current
  OLED::setCursor(infoX, 8);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::SMALL, 2);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  OLED::printNumber(getHandleTemperature(0) / 10, 2, FontStyle::SMALL);
  OLED::printSymbolDeg(FontStyle::SMALL);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  {
    uint32_t ampsX10 = voltX10 ? (x10Watt * 10) / voltX10 : 0;
    OLED::printNumber(ampsX10 / 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolDot, FontStyle::SMALL);
    OLED::printNumber(ampsX10 % 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolAmps, FontStyle::SMALL);
  }

  // Line 2: cartridge resistance + chop frequency
  OLED::setCursor(infoX, 16);
  {
    uint8_t tipRx10 = getTipResistanceX10();
    OLED::printNumber(tipRx10 / 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolDot, FontStyle::SMALL);
    OLED::printNumber(tipRx10 % 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolOhm, FontStyle::SMALL);
  }
#ifdef TIP_CURRENT_LIMIT_CHOP
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  OLED::printNumber((getTipChopFrequencyHzX10() + 5000) / 10000, 2, FontStyle::SMALL);
  OLED::print(SmallSymbolKiloHertz, FontStyle::SMALL);
#endif

  // Line 3: effective power limit (supply / user / hardware / handle derate) + chop duty when chopping
  OLED::setCursor(infoX, 24);
  OLED::print(SmallSymbolMax, FontStyle::SMALL);
  OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::SMALL);
  OLED::print(SmallSymbolWatts, FontStyle::SMALL);
#ifdef TIP_CURRENT_LIMIT_CHOP
  {
    uint16_t duty = getTipChopDutyX256Latched();
    if (duty < 256) {
      OLED::print(SmallSymbolSpace, FontStyle::SMALL);
      OLED::printNumber((duty * 100) / 256, 2, FontStyle::SMALL);
      OLED::print(SmallSymbolPercent, FontStyle::SMALL);
    }
  }
#endif
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
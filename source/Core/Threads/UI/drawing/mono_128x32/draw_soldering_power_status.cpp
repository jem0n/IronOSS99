#include "Buttons.hpp"
#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32

/*
 * Detailed soldering view (right handed; the two blocks swap for left handed):
 *
 *   x 0..59                       x 60..127 (11 cells of the 6x8 TINY font)
 *   +----------------------------+-----------------------------+
 *   | 350°C (8x16)               | 45.3W 20.1V                 |  y 0
 *   |                            | PD 41° 2.3A  src/handle/A   |  y 8
 *   | [######    ] power bar     | 5.5Ω 11kHz   tip R / chop f |  y 16
 *   |  320° 12s   set / sleep    | Max130W 63%  limit / duty   |  y 24
 *   +----------------------------+-----------------------------+
 */
extern TickType_t lastMovementTime;

static void printX10WattsTiny(uint32_t x10Watt) {
  if (x10Watt > 999) {
    // >= 100 W: drop the decimal so it still fits in 4 cells
    OLED::printNumber(x10Watt / 10, 3, FontStyle::TINY);
  } else {
    OLED::printNumber(x10Watt / 10, 2, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(x10Watt % 10, 1, FontStyle::TINY);
  }
  OLED::print(SmallSymbolWatts, FontStyle::TINY);
}

static void drawPowerBar(int16_t x, uint32_t x10Watt, int32_t x10Limit) {
  // 48 px wide, 7 px tall frame at y 16..22, filled proportional to the power vs the limit in force
  const uint8_t width = 46;
  uint32_t      len   = 0;
  if (x10Limit > 0) {
    len = (x10Watt * width) / (uint32_t)x10Limit;
    if (len > width) {
      len = width;
    }
  }
  OLED::drawFilledRect(x, 16, x + width + 1, 16, false);
  OLED::drawFilledRect(x, 22, x + width + 1, 22, false);
  OLED::drawFilledRect(x, 16, x, 22, false);
  OLED::drawFilledRect(x + width + 1, 16, x + width + 1, 22, false);
  if (len) {
    OLED::drawFilledRect(x + 1, 18, x + len, 20, false);
  }
}

#ifndef NO_SLEEP_MODE
static void printCountdownUntilSleepTiny(int sleepThres) {
  TickType_t lastEventTime = lastButtonTime < lastMovementTime ? lastMovementTime : lastButtonTime;
  TickType_t downCount     = sleepThres - xTaskGetTickCount() + lastEventTime;
  if (downCount > (99 * TICKS_SECOND)) {
    OLED::printNumber(downCount / 60000 + 1, 2, FontStyle::TINY);
    OLED::print(SmallSymbolMinutes, FontStyle::TINY);
  } else {
    OLED::printNumber(downCount / 1000 + 1, 2, FontStyle::TINY);
    OLED::print(SmallSymbolSeconds, FontStyle::TINY);
  }
}
#endif

void ui_draw_soldering_power_status(bool boost_mode_on) {
  const bool     leftHanded = OLED::getRotation();
  const int16_t  mainX      = leftHanded ? 68 : 0;
  const int16_t  infoX      = leftHanded ? 0 : 60;
  const uint32_t x10Watt    = x10WattHistory.average();
  const uint32_t voltX10    = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  const int32_t  x10Limit   = getX10WattageLimits();

  // Main block: tip temperature (8x16), power bar, set point + sleep countdown
  OLED::setCursor(mainX, 0);
  ui_draw_tip_temperature(true, FontStyle::SMALL);

  drawPowerBar(mainX, x10Watt, x10Limit);

  OLED::setCursor(mainX, 24);
  if (boost_mode_on) {
    OLED::print(SmallSymbolPlus, FontStyle::TINY);
    OLED::printNumber(getSettingValue(SettingsOptions::BoostTemp), 3, FontStyle::TINY);
  } else {
    OLED::print(SmallSymbolSpace, FontStyle::TINY);
    OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::TINY);
  }
  OLED::printSymbolDeg(FontStyle::TINY);
#ifndef NO_SLEEP_MODE
  if (!boost_mode_on && getSettingValue(SettingsOptions::Sensitivity) && getSettingValue(SettingsOptions::SleepTime)) {
    OLED::setCursor(mainX + 36, 24);
    printCountdownUntilSleepTiny(getSleepTimeout());
  }
#endif

  // Info block, line 0: wattage + input voltage
  OLED::setCursor(infoX, 0);
  printX10WattsTiny(x10Watt);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
  OLED::print(SmallSymbolDot, FontStyle::TINY);
  OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
  OLED::print(SmallSymbolVolts, FontStyle::TINY);

  // Line 1: power source, handle temperature, tip current
  OLED::setCursor(infoX, 8);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, 2);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(getHandleTemperature(0) / 10, 2, FontStyle::TINY);
  OLED::printSymbolDeg(FontStyle::TINY);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  {
    uint32_t ampsX10 = voltX10 ? (x10Watt * 10) / voltX10 : 0;
    OLED::printNumber(ampsX10 / 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(ampsX10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolAmps, FontStyle::TINY);
  }

  // Line 2: cartridge resistance + chop frequency
  OLED::setCursor(infoX, 16);
  {
    uint8_t tipRx10 = getTipResistanceX10();
    OLED::printNumber(tipRx10 / 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(tipRx10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolOhm, FontStyle::TINY);
  }
#ifdef TIP_CURRENT_LIMIT_CHOP
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber((getTipChopFrequencyHzX10() + 5000) / 10000, 2, FontStyle::TINY);
  OLED::print(SmallSymbolKiloHertz, FontStyle::TINY);
#endif

  // Line 3: power limit in force (supply / user / hardware / handle derate) + chop duty when chopping
  OLED::setCursor(infoX, 24);
  OLED::print(SmallSymbolMax, FontStyle::TINY);
  OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::TINY);
  OLED::print(SmallSymbolWatts, FontStyle::TINY);
#ifdef TIP_CURRENT_LIMIT_CHOP
  {
    uint16_t duty = getTipChopDutyX256Latched();
    if (duty < 256) {
      OLED::print(SmallSymbolSpace, FontStyle::TINY);
      OLED::printNumber((duty * 100) / 256, 2, FontStyle::TINY);
      OLED::print(SmallSymbolPercent, FontStyle::TINY);
    }
  }
#endif
}
#endif

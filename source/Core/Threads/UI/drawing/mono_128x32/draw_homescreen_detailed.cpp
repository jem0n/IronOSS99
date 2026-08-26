#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32

extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

/*
 * Idle data view (right handed; blocks swap for left handed):
 *   x 0..59                    x 60..127 (6x8 TINY font)
 *   | 123°C (8x16)            | 41° 5.5Ω   handle / cartridge |  y 0
 *   |                         | Max130W    power limit        |  y 8
 *   |  320°  set point        | 11kHz      chop frequency     |  y 16
 *   | PD 20.1V                |                               |  y 24
 */
void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t mainX      = leftHanded ? 68 : 0;
  const int16_t infoX      = leftHanded ? 0 : 60;

  if (isTipDisconnected()) {
    if (leftHanded) {
      OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
      OLED::setCursor(-1, 4);
    } else {
      OLED::drawArea(0, 0, 56, 32, disconnectedTip);
      OLED::setCursor(56, 4);
    }
    uint32_t Vlt = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(Vlt / 10, 2, FontStyle::LARGE);
    OLED::print(LargeSymbolDot, FontStyle::LARGE);
    OLED::printNumber(Vlt % 10, 1, FontStyle::LARGE);
    OLED::setCursor(OLED::getCursorX(), 12);
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
    return;
  }

  if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (tipTemp > 55) && (xTaskGetTickCount() % 1000 < 300))) {
    // Blink temp if setting enable and temp < 55°
    OLED::setCursor(mainX, 0);
    ui_draw_tip_temperature(true, FontStyle::SMALL);
  }
  OLED::setCursor(mainX, 16);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::TINY);
  OLED::printSymbolDeg(FontStyle::TINY);
  OLED::setCursor(mainX, 24);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, 2);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  {
    uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolVolts, FontStyle::TINY);
  }

  // Info block
  OLED::setCursor(infoX, 0);
  OLED::printNumber(getHandleTemperature(0) / 10, 2, FontStyle::TINY);
  OLED::printSymbolDeg(FontStyle::TINY);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  {
    uint8_t tipRx10 = getTipResistanceX10();
    OLED::printNumber(tipRx10 / 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(tipRx10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolOhm, FontStyle::TINY);
  }
  OLED::setCursor(infoX, 8);
  {
    int32_t x10Limit = getX10WattageLimits();
    OLED::print(SmallSymbolMax, FontStyle::TINY);
    OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::TINY);
    OLED::print(SmallSymbolWatts, FontStyle::TINY);
  }
#ifdef TIP_CURRENT_LIMIT_CHOP
  OLED::setCursor(infoX, 16);
  OLED::printNumber((getTipChopFrequencyHzX10() + 5000) / 10000, 2, FontStyle::TINY);
  OLED::print(SmallSymbolKiloHertz, FontStyle::TINY);
#endif
}
#endif

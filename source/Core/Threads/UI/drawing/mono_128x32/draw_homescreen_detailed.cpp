#include "OperatingModes.h"
#include "power.hpp"
#include "ui_drawing.hpp"
#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t infoX      = leftHanded ? 0 : 60;

  if (isTipDisconnected()) {
    if (leftHanded) {
      OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
      OLED::setCursor(-1, 0);
    } else {
      OLED::drawArea(0, 0, 56, 32, disconnectedTip);
      OLED::setCursor(56, 0);
    }
    uint32_t Vlt = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(Vlt / 10, 2, FontStyle::LARGE);
    OLED::print(LargeSymbolDot, FontStyle::LARGE);
    OLED::printNumber(Vlt % 10, 1, FontStyle::LARGE);
    OLED::setCursor(leftHanded ? 48 : 91, 8);
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
    return;
  }

  /*
   * Idle data view (right handed; blocks swap for left handed):
   *   x 0..59                    x 60..127
   *   | 123° (LARGE)            | 41° 5.5Ω   handle / cartridge |  y 0
   *   |                         | Max130W    effective limit    |  y 8
   *   |  320°  set point        | 11kHz      chop frequency     |  y 16
   *   | PD 20.1V                |                               |  y 24
   */
  const int16_t mainX = leftHanded ? 68 : 0;
  if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (tipTemp > 55) && (xTaskGetTickCount() % 1000 < 300))) {
    // Blink temp if setting enable and temp < 55°
    OLED::setCursor(mainX, 0);
    ui_draw_tip_temperature(true, FontStyle::LARGE);
  }
  OLED::setCursor(mainX, 16);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::SMALL);
  OLED::printSymbolDeg(FontStyle::SMALL);
  OLED::setCursor(mainX, 24);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::SMALL, 2);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);

  // Info block
  OLED::setCursor(infoX, 0);
  OLED::printNumber(getHandleTemperature(0) / 10, 2, FontStyle::SMALL);
  OLED::printSymbolDeg(FontStyle::SMALL);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  {
    uint8_t tipRx10 = getTipResistanceX10();
    OLED::printNumber(tipRx10 / 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolDot, FontStyle::SMALL);
    OLED::printNumber(tipRx10 % 10, 1, FontStyle::SMALL);
    OLED::print(SmallSymbolOhm, FontStyle::SMALL);
  }
  OLED::setCursor(infoX, 8);
  {
    int32_t x10Limit = getX10WattageLimits();
    OLED::print(SmallSymbolMax, FontStyle::SMALL);
    OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::SMALL);
    OLED::print(SmallSymbolWatts, FontStyle::SMALL);
  }
#ifdef TIP_CURRENT_LIMIT_CHOP
  OLED::setCursor(infoX, 16);
  OLED::printNumber((getTipChopFrequencyHzX10() + 5000) / 10000, 2, FontStyle::SMALL);
  OLED::print(SmallSymbolKiloHertz, FontStyle::SMALL);
#endif
}

#else  /* scaled 96x16 layout */

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  if (isTipDisconnected()) {
    if (OLED::getRotation()) {
      // in right handed mode we want to draw over the first part
      OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
    } else {
      OLED::drawArea(0, 0, 56, 32, disconnectedTip);
    }
    if (OLED::getRotation()) {
      OLED::setCursor(-1, 0);
    } else {
      OLED::setCursor(56, 0);
    }
    uint32_t Vlt = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(Vlt / 10, 2, FontStyle::LARGE);
    OLED::print(LargeSymbolDot, FontStyle::LARGE);
    OLED::printNumber(Vlt % 10, 1, FontStyle::LARGE);
    if (OLED::getRotation()) {
      OLED::setCursor(48, 8);
    } else {
      OLED::setCursor(91, 8);
    }
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
  } else {
    if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (tipTemp > 55) && (xTaskGetTickCount() % 1000 < 300))) {
      // Blink temp if setting enable and temp < 55°
      // 1000 tick/sec
      // OFF 300ms ON 700ms
      ui_draw_tip_temperature(true, FontStyle::LARGE); // draw in the temp
    }
    if (OLED::getRotation()) {
      OLED::setCursor(6, 0);
    } else {
      OLED::setCursor(73, 0); // top right
    }
    // draw set temp
    OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::SMALL);

    OLED::printSymbolDeg(FontStyle::SMALL);

    if (OLED::getRotation()) {
      OLED::setCursor(0, 8);
    } else {
      OLED::setCursor(67, 8); // bottom right
    }
    printVoltage(); // draw voltage then symbol (v)
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
  }
}
#endif /* OLED_128x32_HIRES_UI */
#endif
#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32

// [heat / boost column 16px][LARGE temp + °C 48px][info column: 20.0V / source / set point in the 6x8 font]
// The side columns swap over for left handed mode so the heat indicator stays next to the buttons.
void ui_draw_soldering_basic_status(bool boostModeOn) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t heatX      = leftHanded ? 116 : 0;
  const int16_t tempX      = leftHanded ? 62 : 16;
  const int16_t infoX      = leftHanded ? 8 : 70;

  OLED::setCursor(heatX, 0);
  OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
  if (boostModeOn) {
    OLED::setCursor(heatX, 16);
    OLED::drawSymbol(2); // Up arrow
  }

  OLED::setCursor(tempX, 4); // y=4 centres the 24px digits
  ui_draw_tip_temperature(true, FontStyle::LARGE);

  OLED::setCursor(infoX, 4);
  {
    uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolVolts, FontStyle::TINY);
  }
  OLED::setCursor(infoX, 12);
#ifdef POW_DC
  if (getIsPoweredByDCIN() && getSettingValue(SettingsOptions::MinDCVoltageCells)) {
    ui_draw_power_source_icon(); // battery gauge
  } else
#endif
  {
    OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, 2);
  }
  OLED::setCursor(infoX, 20);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(getSettingValue(boostModeOn ? SettingsOptions::BoostTemp : SettingsOptions::SolderingTemp), 3, FontStyle::TINY);
  OLED::printSymbolDeg(FontStyle::TINY);
}
#endif

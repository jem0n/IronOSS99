#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32

// Two full width rows in the 8x16 small font (16 cells each):
//   350°C 45W 20.1V     tip temperature, power, input voltage
//   41°C 5.5Ω <130W     handle temperature, cartridge resistance, power limit currently in force
// Rows read left to right in both orientations so nothing flips with rotation.
void ui_draw_soldering_power_status(bool boost_mode_on) {
  (void)boost_mode_on;
  // Row 0
  OLED::setCursor(0, 0);
  ui_draw_tip_temperature(true, FontStyle::SMALL);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  OLED::printNumber(x10WattHistory.average() / 10, 3, FontStyle::SMALL);
  OLED::print(SmallSymbolWatts, FontStyle::SMALL);
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  printVoltage();
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);

  // Row 1
  OLED::setCursor(0, 16);
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
  OLED::print(SmallSymbolSpace, FontStyle::SMALL);
  {
    int32_t x10Limit = getX10WattageLimits();
    OLED::print(SmallSymbolLessThan, FontStyle::SMALL);
    OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::SMALL);
    OLED::print(SmallSymbolWatts, FontStyle::SMALL);
  }
}
#endif

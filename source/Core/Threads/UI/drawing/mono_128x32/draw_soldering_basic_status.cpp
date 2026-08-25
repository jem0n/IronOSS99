#include "power.hpp"
#include "ui_drawing.hpp"
#ifdef OLED_128x32
#ifdef OLED_128x32_HIRES_UI

// Full height layout:  [heat / boost column 16px][HUGE temp + deg symbol 96px][power source column 16px]
// The two side columns swap over for left handed mode so the heat indicator stays next to the buttons.
void ui_draw_soldering_basic_status(bool boostModeOn) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t heatX      = leftHanded ? 116 : 0;
  const int16_t sourceX    = leftHanded ? 0 : 116;

  OLED::setCursor(heatX, 0);
  OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
  if (boostModeOn) {
    OLED::setCursor(heatX, 16);
    OLED::drawSymbol(2); // Up arrow
  }

  OLED::setCursor(16, 0);
  ui_draw_tip_temperature(true, FontStyle::HUGE);

  OLED::setCursor(sourceX, 0);
  ui_draw_power_source_icon();
}

#else  /* scaled 96x16 layout */

void ui_draw_soldering_basic_status(bool boostModeOn) {
  OLED::setCursor(0, 0);
  // We switch the layout direction depending on the orientation of the oled
  if (OLED::getRotation()) {
    // battery
    ui_draw_power_source_icon();
    // Space out gap between battery <-> temp
    OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    // Draw current tip temp
    ui_draw_tip_temperature(true, FontStyle::LARGE);

    // We draw boost arrow if boosting,
    // or else gap temp <-> heat indicator
    if (boostModeOn) {
      OLED::drawSymbol(2);
    } else {
      OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    }

    // Draw heating/cooling symbols
    OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
  } else {
    // Draw heating/cooling symbols
    OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
    // We draw boost arrow if boosting,
    // or else gap temp <-> heat indicator
    if (boostModeOn) {
      OLED::drawSymbol(2);
    } else {
      OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    }
    // Draw current tip temp
    ui_draw_tip_temperature(true, FontStyle::LARGE);
    // Space out gap between battery <-> temp
    OLED::print(LargeSymbolSpace, FontStyle::LARGE);

    ui_draw_power_source_icon();
  }
}
#endif /* OLED_128x32_HIRES_UI */
#endif
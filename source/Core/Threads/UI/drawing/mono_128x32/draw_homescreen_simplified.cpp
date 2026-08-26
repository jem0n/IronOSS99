#include "ui_drawing.hpp"

#ifdef OLED_128x32

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

/*
 * [button A 0..52][button B 50..102][ 20.0 / V  104..127 ]
 * The round pictograms only use columns 0..52 of their 56 px bitmaps and their outer columns are
 * blank except for the little pointer nub, so button B can sit 8 px further left without touching A.
 * That leaves 24 px for the input voltage in the 6x8 font instead of two stacked digits.
 */
static void drawPowerSourceSmall(int16_t x) {
#ifdef POW_DC
  if (getIsPoweredByDCIN() && getSettingValue(SettingsOptions::MinDCVoltageCells)) {
    // Lithium pack: keep the battery gauge
    OLED::setCursor(x + 6, 8);
    ui_draw_power_source_icon();
    return;
  }
#endif
  uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  OLED::setCursor(x, 8);
  OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
  OLED::print(SmallSymbolDot, FontStyle::TINY);
  OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
  OLED::setCursor(x + 9, 16);
  OLED::print(SmallSymbolVolts, FontStyle::TINY);
}

void ui_draw_homescreen_simplified(TemperatureType_t tipTemp) {
  const bool    leftHanded             = OLED::getRotation();
  const int16_t buttonAX               = leftHanded ? 71 : 0;
  const int16_t buttonBX               = leftHanded ? 21 : 50;
  const int16_t sourceX                = leftHanded ? 0 : 104;
  bool          tempOnDisplay          = false;
  bool          tipDisconnectedDisplay = false;

  OLED::drawArea(buttonAX, 0, 56, 32, leftHanded ? buttonAF : buttonA);
  OLED::drawArea(buttonBX, 0, 56, 32, leftHanded ? buttonBF : buttonB);
  drawPowerSourceSmall(sourceX);

  if (tipTemp > 55) {
    tempOnDisplay = true;
  } else if (tipTemp < 45) {
    tempOnDisplay = false;
  }
  if (isTipDisconnected()) {
    tempOnDisplay          = false;
    tipDisconnectedDisplay = true;
  }
  if (tempOnDisplay || tipDisconnectedDisplay) {
    // Draw the temp (or the missing tip symbol) over the start soldering button.
    // Clear the whole A pictogram (left handed: it is drawn 3 px further right inside its bitmap)
    OLED::fillArea(leftHanded ? 74 : 0, 0, leftHanded ? 54 : 50, 32, 0);
    if (!tipDisconnectedDisplay) {
      if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300))) {
        OLED::setCursor(leftHanded ? 78 : 2, 4); // y=4 centres the 24px digits
        ui_draw_tip_temperature(true, FontStyle::LARGE);
      }
    } else {
      OLED::drawArea(buttonAX, 0, 56, 32, leftHanded ? disconnectedTipF : disconnectedTip);
    }
  }
}
#endif

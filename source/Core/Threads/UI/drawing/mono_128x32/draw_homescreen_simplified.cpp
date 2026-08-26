#include "ui_drawing.hpp"

#ifdef OLED_128x32

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

#ifdef OLED_128x32_HIRES_UI
/*
 * [button A 0..52][button B 50..102][ 20.0 / V  104..127 ]
 * The round pictograms only use columns 0..52 of their 56 px bitmaps and their outer columns are
 * blank except for the little pointer nub, so button B can sit 8 px further left without touching A.
 * That leaves 24 px for the input voltage in the small font instead of two stacked large digits.
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
  OLED::setCursor(x, 8);
  printVoltage();
  OLED::setCursor(x + 9, 16);
  OLED::print(SmallSymbolVolts, FontStyle::SMALL);
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
    // Draw the temp (or the missing tip symbol) over the start soldering button
    // Clear the whole A pictogram (left handed: it is drawn 3 px further right inside its bitmap)
    OLED::fillArea(leftHanded ? 74 : 0, 0, leftHanded ? 54 : 50, 32, 0);
    if (!tipDisconnectedDisplay) {
      if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300))) {
        OLED::setCursor(leftHanded ? 78 : 4, 8);
        ui_draw_tip_temperature(true, FontStyle::LARGE);
      }
    } else {
      OLED::drawArea(buttonAX, 0, 56, 32, leftHanded ? disconnectedTipF : disconnectedTip);
    }
  }
}

#else /* scaled 96x16 layout */

void ui_draw_homescreen_simplified(TemperatureType_t tipTemp) {
  bool tempOnDisplay          = false;
  bool tipDisconnectedDisplay = false;
  if (OLED::getRotation()) {
    OLED::drawArea(68, 0, 56, 32, buttonAF);
    OLED::drawArea(12, 0, 56, 32, buttonBF);
    OLED::setCursor(0, 0);
    ui_draw_power_source_icon();
  } else {
    OLED::drawArea(0, 0, 56, 32, buttonA);  // Needs to be flipped so button ends up
    OLED::drawArea(58, 0, 56, 32, buttonB); // on right side of screen
    OLED::setCursor(116, 0);
    ui_draw_power_source_icon();
  }
  tipDisconnectedDisplay = false;
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
    // draw temp over the start soldering button
    // Location changes on screen rotation
    if (OLED::getRotation()) {
      // in right handed mode we want to draw over the first part
      OLED::fillArea(68, 0, 56, 32, 0); // clear the area for the temp
      OLED::setCursor(56, 0);
    } else {
      OLED::fillArea(0, 0, 56, 32, 0); // clear the area
      OLED::setCursor(0, 0);
    }
    // If we have a tip connected draw the temp, if not we leave it blank
    if (!tipDisconnectedDisplay) {
      // draw in the temp
      if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300))) {
        ui_draw_tip_temperature(false, FontStyle::LARGE); // draw in the temp
      }
    } else {
      // Draw in missing tip symbol
      if (OLED::getRotation()) {
        // in right handed mode we want to draw over the first part
        OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
      } else {
        OLED::drawArea(0, 0, 56, 32, disconnectedTip);
      }
    }
  }
}

#endif /* OLED_128x32_HIRES_UI */
#endif

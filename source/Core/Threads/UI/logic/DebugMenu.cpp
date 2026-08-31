#include "OperatingModes.h"
#include "ui_drawing.hpp"

OperatingMode showDebugMenu(const ButtonState buttons, guiContext *cxt) {

  ui_draw_debug_menu(cxt->scratch_state.state1);

  if (buttons == BUTTON_B_SHORT) {
    cxt->transitionMode = TransitionAnimation::Up;
    return OperatingMode::HomeScreen;
  } else if (buttons == BUTTON_F_SHORT) {
    cxt->scratch_state.state1++;
    uint16_t entries = 16;
#ifdef HALL_SENSOR
    entries = 17;
#endif
#ifdef MCU_TEMP_CUTOFF_C
    entries = 18; // the die temperature is the last entry
#endif
    cxt->scratch_state.state1 = cxt->scratch_state.state1 % entries;
  }
  return OperatingMode::DebugMenuReadout; // Stay in debug menu
}

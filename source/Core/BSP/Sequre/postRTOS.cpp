#include "BSP.h"
// #include "Pins.h"

// Initialisation to be performed with scheduler active
// void postRToSInit() {
// #ifdef BUZZER_Pin
  // Short chirp at boot (the stock firmware plays a start-up melody); silent when the buzzer setting is off
  // setBuzzer(true);
  // delay_ms(80);
  // setBuzzer(false);
// #endif
// }

// Initialisation to be performed with scheduler active.
// Note: this runs at the start of the power thread, right before the USB-PD negotiation is kicked off, so it must
// not block - the start-up chirp lives in the GUI thread (after the boot logo) for that reason.
void postRToSInit() {}

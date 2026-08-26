#include "BSP.h"
#include "Pins.h"

// Initialisation to be performed with scheduler active
void postRToSInit() {
#ifdef BUZZER_Pin
  // Short chirp at boot (the stock firmware plays a start-up melody); silent when the buzzer setting is off
  setBuzzer(true);
  delay_ms(80);
  setBuzzer(false);
#endif
}

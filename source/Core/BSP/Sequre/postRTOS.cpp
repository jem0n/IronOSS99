#include "BSP.h"
#include "Pins.h"

// Initialisation to be performed with scheduler active
void postRToSInit() {
#ifdef BUZZER_Pin
  // Short chirp at boot (the stock firmware plays a start-up melody), also handy to verify the buzzer
  setBuzzer(true);
  delay_ms(80);
  setBuzzer(false);
#endif
}

#include "LED_MACROS.h"

LED_MACROS LED;

void setup() {
  LED.SetPin(3);
}

void loop() {
 LED.Run();
}

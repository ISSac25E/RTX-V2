#include "LED_MACROS.h"

LED_MACROS LED;

void setup() {
  LED.SetPin(3);
  Serial.begin(115200);
}

void loop() {
  Serial.println(LED.PWM());
  if(LED.Run()) {
    switch(LED.Last()) {
      case 0:
      LED.Fade(100, 6);
      break;
      case 1:
      LED.Fade(30, 6);
      break;
      case 2:
      LED.Set(30, 50);
      break;
      case 3:
      LED.Set(100, 50);
      break;
    }
  }
}

 #include "LED_MACROS.h"

LED_MACROS LED(3);

void setup() {
  //Only PWM Pins: 3, 5, 6, 9, 10, 11
//  LED.SetPin(3);
  Serial.begin(115200);
}

void loop() {
  Serial.println(LED.PWM());
//  if(LED.Run()) {
//    switch(LED.Mode) {
//      case 0:
//      LED.Fade(100, 6);
//      LED.Mode = 3;
//      break;
//      case 1:
//      LED.Fade(20, 6);
//      LED.Mode = 2;
//      break;
//      case 2:
//      LED.Set(20, 50);
//      LED.Mode = 0;
//      break;
//      case 3:
//      LED.Set(100, 50);
//      LED.Mode = 1;
//      break;
//    }
//  }

if(LED.Run()) {
    switch(LED.Mode) {
      case 0:
      LED.Set(100, 50);
      LED.Mode = 1;
      break;
      case 1:
      LED.Set(0, 100);
      LED.Mode = 2;
      break;
      case 2:
      LED.Set(100, 50);
      LED.Mode = 3;
      break;
      case 3:
      LED.Set(0, 1000);
      LED.Mode = 0;
      break;
    }
  }
}

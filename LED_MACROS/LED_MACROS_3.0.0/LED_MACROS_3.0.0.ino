#include "LED_MACROS.h"


LED_MACROS LED1(10);
LED_MACROS LED2(11);
MACROS_BUILD LedMacro_1;

void setup() {
  //Only PWM Pins: 3, 5, 6, 9, 10, 11
  //  LED.SetPin(3);
  LedMacro_1.Macro = 1;
  Serial.begin(115200);
}

void loop() {
  LedRun();

}

void LedRun() {
  switch (LedMacro_1.Macro) {
    case 0:
      LedMacro_1.PrevMacro = 0;
      break;
    case 1:
      if (LedMacro_1.PrevMacro != 1) {
        LED1.Set(0, 5000);
        LED2.SetPWM(0);
        LedMacro_1.MacroStage = 0;
        LedMacro_1.PrevMacro = 1;
      }
      if (LED1.Run() && LED2.Run()) {
        switch (LedMacro_1.MacroStage) {
          case 0:
            LED1.Set(100, 50);
            LedMacro_1.MacroStage = 1;
            break;
          case 1:
            LED1.Set(0, 50);
            LedMacro_1.MacroStage = 2;
            break;
          case 2:
            LED2.Set(100, 50);
            LedMacro_1.MacroStage = 3;
            break;
          case 3:
            LED2.Set(0, 50);
            LedMacro_1.MacroStage = 4;
            break;
          case 4:
            LED1.Set(100, 50);
            LedMacro_1.MacroStage = 5;
            break;
          case 5:
            LED1.Set(0, 50);
            LedMacro_1.MacroStage = 6;
            break;
          case 6:
            LED2.Set(100, 50);
            LedMacro_1.MacroStage = 7;
            break;
          case 7:
          LED2.Set(0, 1000);
            LedMacro_1.MacroStage = 0;
            break;
        }
      }
      break;
    case 2:
      break;
  }
}

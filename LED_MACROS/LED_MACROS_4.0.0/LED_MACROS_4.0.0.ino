#include "LED_MACROS.h"
LED_DRIVE LED1(10);

MACROS MACROS1;
MACROS MACROS2;
MACROS_BUILD MACROS_B1;
MACROS_BUILD MACROS_B2;

//LED_MACROS LED1(10);
//LED_MACROS LED2(11);
//MACROS_BUILD LedMacro_1;
//MACROS_BUILD LedMacro_0;


void setup() {
  pinMode(3, INPUT_PULLUP);
  //Only PWM Pins: 3, 5, 6, 9, 10, 11
  //  LED.SetPin(3);
  //  LED1.SetFPS(60);
  //  LED2.SetFPS(60);
  //  LedMacro_1.Macro = 1;
  //  LedMacro_0.Macro = 0;
  MACROS_B1.Macro = 1;
  MACROS_B2.Macro = 1;
  MACROS1.SetFPS(60);
  MACROS2.SetFPS(60);
  Serial.begin(115200);
}

void loop() {
  LedRun2();

}

void LedRun2() {
  if (digitalRead(3))
    LED1.PWM(MACROS1.Val());
  else
    LED1.PWM(MACROS2.Val());
    
  switch (MACROS_B2.Macro) {
    case 1:
      if (MACROS_B2.PrevMacro != MACROS_B2.Macro) {
        MACROS1.Fade(20, 6);
        MACROS_B2.MacroStage = 0;
        MACROS_B2.PrevMacro = MACROS_B2.Macro;
      }
      if (MACROS2.Run()) {
        switch (MACROS_B2.MacroStage) {
          case 0:
            MACROS2.Fade(100, 6);
            MACROS_B2.MacroStage = 1;
            break;
          case 1:
            MACROS2.SetDelay(50);
            MACROS_B2.MacroStage = 2;
            break;
          case 2:
            MACROS2.Fade(20, 6);
            MACROS_B2.MacroStage = 3;
            break;
          case 3:
            MACROS2.SetDelay(50);
            MACROS_B2.MacroStage = 0;
            break;
        }
      }
      break;
  }

  switch (MACROS_B1.Macro) {
    case 1:
      if (MACROS_B1.PrevMacro != MACROS_B1.Macro) {
        MACROS1.Fade(20, 6);
        MACROS_B1.MacroStage = 0;
        MACROS_B1.PrevMacro = MACROS_B1.Macro;
      }
      if (MACROS1.Run()) {
        switch (MACROS_B1.MacroStage) {
          case 0:
            MACROS1.Fade(100, 12);
            MACROS_B1.MacroStage = 1;
            break;
          case 1:
            MACROS1.SetDelay(50);
            MACROS_B1.MacroStage = 2;
            break;
          case 2:
            MACROS1.Fade(20, 12);
            MACROS_B1.MacroStage = 3;
            break;
          case 3:
            MACROS1.SetDelay(50);
            MACROS_B1.MacroStage = 0;
            break;
        }
      }
      break;
  }
}

//void LedRun() {
//  switch (LedMacro_0.Macro) {
//    case 0:
//      LedMacro_0.PrevMacro = 0;
//      break;
//    case 1:
//      if (LedMacro_0.PrevMacro != LedMacro_0.Macro) {
//        LED2.Fade(20, 6);
//        LedMacro_0.MacroStage = 0;
//        LedMacro_0.PrevMacro = LedMacro_0.Macro;
//      }
//      LED2.Run();
//      if (LED2.Ready()) {
//        switch (LedMacro_0.MacroStage) {
//          case 0:
//            LED2.Fade(20, 12);
//            LedMacro_0.MacroStage = 1;
//            break;
//          case 1:
//            LED2.SetDelay(100);
//            LedMacro_0.MacroStage = 2;
//            break;
//          case 2:
//            LED2.Fade(100, 12);
//            LedMacro_0.MacroStage = 3;
//            break;
//          case 3:
//            LED2.SetDelay(100);
//            LedMacro_0.MacroStage = 0;
//            break;
//        }
//      }
//      break;
//    case 2:
//      break;
//    case 3:
//      break;
//  }
//  switch (LedMacro_1.Macro) {
//    case 0:
//      LedMacro_1.PrevMacro = 0;
//      break;
//    case 1:
//      if (LedMacro_1.PrevMacro != 1) {
//        LED1.Set(0, 5000);
//        LED2.SetPWM(0);
//        LedMacro_1.MacroStage = 0;
//        LedMacro_1.PrevMacro = 1;
//      }
//      LED1.Run();
//      LED2.Run();
//      if (LED1.Ready() && LED2.Ready()) {
//        switch (LedMacro_1.MacroStage) {
//          case 0:
//            LED1.Set(100, 50);
//            LedMacro_1.MacroStage = 1;
//            break;
//          case 1:
//            LED1.Set(0, 50);
//            LedMacro_1.MacroStage = 2;
//            break;
//          case 2:
//            LED2.Set(100, 50);
//            LedMacro_1.MacroStage = 3;
//            break;
//          case 3:
//            LED2.Set(0, 50);
//            LedMacro_1.MacroStage = 4;
//            break;
//          case 4:
//            LED1.Set(100, 50);
//            LedMacro_1.MacroStage = 5;
//            break;
//          case 5:
//            LED1.Set(0, 50);
//            LedMacro_1.MacroStage = 6;
//            break;
//          case 6:
//            LED2.Set(100, 50);
//            LedMacro_1.MacroStage = 7;
//            break;
//          case 7:
//            LED2.Set(0, 1000);
//            LedMacro_1.MacroStage = 0;
//            break;
//        }
//      }
//      break;
//    case 2:
//      if (LedMacro_1.PrevMacro != 2) {
//        LED1.Fade(0, 6);
//        LED2.Fade(0, 6);
//        LedMacro_1.MacroStage = 0;
//        LedMacro_1.PrevMacro = 2;
//      }
//      LED1.Run();
//      LED2.Run();
//      if (LED1.Ready() && LED2.Ready()) {
//        switch (LedMacro_1.MacroStage) {
//          case 0:
//            LED1.Fade(100, 60);
//            LED2.Fade(20, 60);
//            LedMacro_1.MacroStage = 1;
//            break;
//          case 1:
//            LED1.SetDelay(200);
//            LED2.SetDelay(200);
//            LedMacro_1.MacroStage = 2;
//            break;
//          case 2:
//            LED1.Fade(20, 60);
//            LED2.Fade(100, 60);
//            LedMacro_1.MacroStage = 3;
//            break;
//          case 3:
//            LED1.SetDelay(200);
//            LED2.SetDelay(200);
//            LedMacro_1.MacroStage = 0;
//            break;
//        }
//      }
//      break;
//    case 3:
//      if (LedMacro_1.PrevMacro != LedMacro_1.Macro) {
//        LED1.Set(0, 100);
//        LedMacro_1.MacroStage = 0;
//        LedMacro_1.PrevMacro = LedMacro_1.Macro;
//      }
//      LED1.Run();
//      if (LED1.Ready()) {
//        switch (LedMacro_1.MacroStage) {
//          case 0:
//            LED1.Set(100, 50);
//            LedMacro_1.MacroStage = 1;
//            break;
//          case 1:
//            LED1.Set(0, 50);
//            LedMacro_1.MacroStage = 2;
//            break;
//          case 2:
//            LED1.Set(100, 50);
//            LedMacro_1.MacroStage = 3;
//            break;
//          case 3:
//            LED1.Set(0, 1000);
//            LedMacro_1.MacroStage = 0;
//            break;
//        }
//      }
//      break;
//  }
//}

#include "BUTTON_INTERFACE.h"

PIN_DRIVER Button[3] = {(2), (3), (4)};
PIN_MACRO PinMacro[3];

void setup() {
  Serial.begin(115200);
  Serial.println("INIT");
  // pinMode(6, OUTPUT);
  //  Button.ButtonDebounce(0);
}

void loop() {
  if (PinMacro[0].Run(Button[0].Run())) {
    if (PinMacro[0].State()) {
      Serial.print("0|>> :");
      Serial.println(PinMacro[0].PrevInterval());
    }
    else {
      Serial.print("0|<< :");
      Serial.println(PinMacro[0].PrevInterval());
    }
  }

  if (PinMacro[1].Run(Button[1].Run())) {
    if (PinMacro[1].State()) {
      Serial.print("1|>> :");
      Serial.println(PinMacro[1].PrevInterval());
    }
    else {
      Serial.print("1|<< :");
      Serial.println(PinMacro[1].PrevInterval());
    }
  }

  if (PinMacro[2].Run(Button[2].Run())) {
    if (PinMacro[2].State()) {
      Serial.print("2|>> :");
      Serial.println(PinMacro[2].PrevInterval());
    }
    else {
      Serial.print("2|<< :");
      Serial.println(PinMacro[2].PrevInterval());
    }
  }

  
  
  //  if(!Button.ButtonState())Serial.println(Button.ButtonCurrInterval());
}

// bool TestButton() {
//   Button[0].Run();
//   Button[1].Run();
//   if(!Button[0].ButtonState() || !Button[1].ButtonState()) return false;
//   else return true;
// }

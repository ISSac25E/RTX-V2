#include "BUTTON_INTERFACE.h"

PIN_DRIVER Button[] = {(3), (4)};
PIN_MACRO PinMacro;

void setup() {
  Serial.begin(115200);
  Serial.println("INIT");
  pinMode(6, OUTPUT);
  //  Button.ButtonDebounce(0);
}

void loop() {
  if (PinMacro.Run(TestButton())) {
    if (PinMacro.State()) {
      Serial.print("|>> :");
      Serial.println(PinMacro.PrevInterval());
    }
    else {
      Serial.print("|<< :");
      Serial.println(PinMacro.PrevInterval());
    }
  }
  //  if(!Button.ButtonState())Serial.println(Button.ButtonCurrInterval());
}

bool TestButton() {
  Button[0].Run();
  Button[1].Run();
  if(!Button[0].ButtonState() || !Button[1].ButtonState()) return false;
  else return true;
}

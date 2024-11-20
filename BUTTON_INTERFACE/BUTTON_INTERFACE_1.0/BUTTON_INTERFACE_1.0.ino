#include "BUTTON_INTERFACE.h"

void ButtonChange();
BUTTON_INTERFACE Button(3, ButtonChange);

void setup() {
  Serial.begin(115200);
  Serial.println("INIT");
}

void loop() {
  Button.Run();
//  if(!Button.ButtonState())Serial.println(Button.ButtonCurrInterval());
}

void ButtonChange() {
  Serial.print(">>>>>>>");
  Serial.println(Button.ButtonPrevInterval());
}

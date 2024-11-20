#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\InputMacro\InputMacro_1.0.0.h"
// include pinDriver as well for example:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\PinDriver\PinDriver_1.0.0.h"

PinDriver pin(3);          // set digital pin 3. pullup Mode automatic
InputMacro pinMacro(true); // set start state as high because pin is pullup

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  if (pinMacro(pin))
  {
    if (pinMacro)
    {
      Serial.print("|>> : ");
      Serial.println(pinMacro.prevInterval());
    }
    else
    {
      Serial.print("|<< : ");
      Serial.println(pinMacro.prevInterval());
    }
  }
  if (Serial.available())
  {
    while (Serial.available())
      Serial.read();
    Serial.print("||| : ");
    Serial.println(pinMacro.interval());
  }
}

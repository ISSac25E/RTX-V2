#include <CORE.h>
#include "Projects/General/VarPar/VarPar_1.0.0.h"

// Bool is really the only useful one, For now:
Par_bool Bool = false;
Par_uint32_t Val;

void setup()
{
  Serial.begin(115200);
  Serial.println("INIT");
  Serial.println("\nBOOL:");
  Serial.print("Val: ");
  Serial.println(Bool);
  Bool = true;
  Serial.print("Val: ");
  Serial.println(Bool);
  Serial.print("1st Change: ");
  Serial.println(Bool.change()); // will return true
  Serial.print("2nd Change: ");
  Serial.println(Bool.change()); // will return false
  Serial.println("\nuint32_t:");
  Serial.print("Val:");
  Serial.println(Val);
  Val = Val + 23;
  Serial.print("Change: ");
  Serial.println(Val.change()); // will return true
  Serial.print("Val:");
  Serial.println(Val);
}
void loop()
{
}
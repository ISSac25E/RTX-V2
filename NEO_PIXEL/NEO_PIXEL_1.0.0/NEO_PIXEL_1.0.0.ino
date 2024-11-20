#include "NEO_PIXEL.h"
NEO_PIXEL PIXEL(12, 1);
void setup() {
  // put your setup code here, to run once:
  delay(5);
PIXEL.Write(50, 0, 0);
//delayMicroseconds(5);
//PIXEL.Write(0, 0, 0);
//PIXEL.Write(0, 0, 100);
}

void loop() {
  // put your main code here, to run repeatedly:
PIXEL.Write(50, 0, 0);
delayMicroseconds(500);
}

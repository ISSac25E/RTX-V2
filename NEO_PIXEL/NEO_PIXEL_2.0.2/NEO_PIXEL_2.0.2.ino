#include "NEO_PIXEL.h"
NEO_PIXEL_DRIVER PixelDriver(12);

NEO_PIXEL G(&PixelDriver);
NEO_PIXEL R(&PixelDriver);
NEO_PIXEL B(&PixelDriver);

NEO_PIXEL GG(&PixelDriver, 3);
NEO_PIXEL RR(&PixelDriver);
NEO_PIXEL BB(&PixelDriver);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  PixelDriver.InitPixel();
  G.PWM(10);
  R.PWM(10);
  B.PWM(10);

  GG.PWM(100);
  RR.PWM(100);
  BB.PWM(100);
  Serial.println(G.PWM());
  Serial.println(R.PWM());
  Serial.println(B.PWM());
  //delayMicroseconds(5);
  //PIXEL.Write(0, 0, 0);
  //PIXEL.Write(0, 0, 100);
}

void loop() {
  // put your main code here, to run repeatedly:
  //PIXEL.Write(50, 0, 0);
  //delayMicroseconds(500);
}

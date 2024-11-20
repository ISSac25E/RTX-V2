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
  G.Write(10);
  R.Write(10);
  B.Write(10);

  GG.Write(100);
  RR.Write(100);
  BB.Write(100);
  Serial.println(G.Val());
  Serial.println(R.Val());
  Serial.println(B.Val());
  //delayMicroseconds(5);
  //PIXEL.Write(0, 0, 0);
  //PIXEL.Write(0, 0, 100);
}

void loop() {
  // put your main code here, to run repeatedly:
  //PIXEL.Write(50, 0, 0);
  //delayMicroseconds(500);
}

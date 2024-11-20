#include "LED_MACROS.h"
#include "NEO_PIXEL.h"

#define BRIGHTNESS 0.10

#define MODE0_R 1.00
#define MODE0_G 1.00
#define MODE0_B 0

#define MODE1_R 0
#define MODE1_G 1.00
#define MODE1_B 0

#define MODE2_R 1.00
#define MODE2_G 0.50
#define MODE2_B 0

NEO_PIXEL_DRIVER PIXEL_DRIVER(12);

NEO_PIXEL G_PIXEL(&PIXEL_DRIVER);
NEO_PIXEL R_PIXEL(&PIXEL_DRIVER);
NEO_PIXEL B_PIXEL(&PIXEL_DRIVER);

uint8_t Mode = 0;

void setup() {
  pinMode(3, INPUT_PULLUP);
  PIXEL_DRIVER.InitPixel();
}

void loop() {
  LedRun();
}

void LedRun() {
  if (!digitalRead(3)) {
    Mode++;
    while (!digitalRead(3));
  }
  switch (Mode % 3) {
    case 0:
    R_PIXEL.Set(((255 * BRIGHTNESS) * MODE0_R));
    G_PIXEL.Set(((255 * BRIGHTNESS) * MODE0_G));
    B_PIXEL.Set(((255 * BRIGHTNESS) * MODE0_B));
      break;
    case 1:
    R_PIXEL.Set(((255 * BRIGHTNESS) * MODE1_R));
    G_PIXEL.Set(((255 * BRIGHTNESS) * MODE1_G));
    B_PIXEL.Set(((255 * BRIGHTNESS) * MODE1_B));
      break;
    case 2:
    R_PIXEL.Set(((255 * BRIGHTNESS) * MODE2_R));
    G_PIXEL.Set(((255 * BRIGHTNESS) * MODE2_G));
    B_PIXEL.Set(((255 * BRIGHTNESS) * MODE2_B));
      break;
  }
  PIXEL_DRIVER.Write();
}

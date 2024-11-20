#define RTX_DEVICE_NAME "TEST"
#define RTX_SOFTWARE_VERSION {1,3,12}
#define RTX_DEVICE_IP 1
#define RTX_EEPROM_SECTORS 32
#define RTX_DEBUG_SECTORS 24

#define EEPROM_WRITE_LED_EN
#define EEPROM_WRITE_LED_PIN 13

#include "RTX_PROTOCAL.h"

void setup() {
  Serial.begin(115200);
  for(uint16_t X = 0; X < 1024; X++) {
    Serial.print(X);
    Serial.print(": ");
    Serial.println(EEPROM.read(X));
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  RTX_PROTOCAL.Run();
}

#include "ATEM_UIP.h"

ATEM_UIP ATEM;

byte ARD_MAC[] = {0x98, 0xA6, 0xAA, 0x15, 0xE8, 0xD9};
IPAddress ARD_IP(192, 168, 86, 153);

void setup() {
  Serial.begin(115200);
  ATEM.Setup(IPAddress(192,168,86,68), 56321);
  Ethernet.begin(ARD_MAC, ARD_IP);
}

void loop() {
  Serial.print(ATEM.Run());
  Serial.print(" ");
  for(uint8_t X = 0; X < 8; X++) {
    Serial.print(ATEM.TallyInput[X]);
  }
  Serial.println();
}

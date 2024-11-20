#include "ATEM_UIP_SLOW.h"

ATEM_UIP_SLOW ATEM;

byte ARD_MAC[] = {0x9F, 0xA1, 0xA3, 0x15, 0xD8, 0x59};
IPAddress ARD_IP(192, 168, 86, 153);

void setup() {
  Serial.begin(115200);
  Serial.print("SERIAL:");
  ATEM.Setup(IPAddress(192, 168, 86, 68), 56321);
  Ethernet.begin(ARD_MAC, ARD_IP);
}

void loop() {
  ATEM.Run();
  if (Serial.available()) {
    uint8_t CMD_Packet[] = {0, 0xCA, 0xE3, 0x82};
    ATEM.SendCommand("DAut", CMD_Packet, 4);
    while(Serial.available()) Serial.read();
  }
  //  Serial.print(ATEM.Run());
  //  Serial.print(" ");
  //  for(uint8_t X = 0; X < 8; X++) {
  //    Serial.print(ATEM.TallyInput[X]);
  //  }
  //  Serial.println();
}

#include "ATEM_UIP.h"

ATEM_UIP ATEM;

byte ARD_MAC[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ARD_IP(192, 168, 86, 44);

void setup() {
  Serial.begin(115200);
  Serial.println("SERIAL:");
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


#include "ATEM_UIP_HYBRID.h"

ATEM_UIP_HYBRID ATEM;

IPAddress ATEM_IP = IPAddress(192,168,86,68);

byte ARD_MAC[] = {0x9F, 0xA1, 0xA3, 0x15, 0xD8, 0x59};
IPAddress ARD_IP(192, 168, 86, 153);

uint16_t ATEM_Port = 56321;

byte PrevTallyInput[8];

uint8_t PrevConn;

void setup() {
  Serial.begin(115200);
  Serial.println("INTI");
  Serial.print("ATEM IP: ");
  Serial.println(ATEM_IP);
  Serial.print("ARDUINO IP: ");
  Serial.println(ARD_IP);
  Serial.print("ATEM UDP PORT: ");
  Serial.println(ATEM_Port);
  Serial.println();
  Serial.println("UDP BEGIN");

  ATEM.Setup(ATEM_IP, ATEM_Port, 8825);
  Ethernet.begin(ARD_MAC, ARD_IP);
}

void loop() {
  uint8_t Conn = ATEM.Run();
  bool TallyChange = false;
  for(uint8_t X = 0; X < 8; X++) {
    if(PrevTallyInput[X] != ATEM.TallyInput[X]) TallyChange = true;
    PrevTallyInput[X] = ATEM.TallyInput[X];
  }
  if(TallyChange) {
    for(uint8_t X = 0; X < 8; X++) {
      Serial.print(ATEM.TallyInput[X]);
      if (X < 7) Serial.print(',');
    }
    Serial.println();
  }
if(PrevConn != Conn) {
  PrevConn = Conn;
  Serial.println(Conn);
}
  
}

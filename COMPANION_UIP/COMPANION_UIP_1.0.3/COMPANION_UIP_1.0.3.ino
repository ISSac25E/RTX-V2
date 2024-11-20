uu#include "COMPANION_UIP.h"
COMP_UIP COMP;
byte ARD_MAC[] = {0x98, 0xA6, 0xAA, 0x15, 0xE8, 0xD9};
IPAddress ARD_IP(192, 168, 86, 177);
void setup() {
  Ethernet.begin(ARD_MAC, ARD_IP);
  COMP.Setup(IPAddress(192,168,86,138), 8000, 8888);
  Serial.begin(115200);
//  COMP.ConnectionSleep(10);
}

void loop() {
  Serial.println(COMP.Run());
  // put your main code here, to run repeatedly:

}

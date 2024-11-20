#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\FSCC\RTX Systems\RTX V2\CORE_Code\COMPANION_UIP\COMPANION_UIP_1.1.0b\include.h"

COMP_UIP COMP;
byte ARD_MAC[] = {0x98, 0xA6, 0xAA, 0x15, 0xE8, 0xD9};
IPAddress ARD_IP(192, 168, 0, 177);

InputMacro CompanionConnection(false);

void setup()
{
  UIPEthernet.begin(ARD_MAC, ARD_IP);
  COMP.Setup(IPAddress(192, 168, 0, 134), 16759, 8888 /* <-- Doesn't matter for this one*/);
  COMP.SetConnectButton(1, 8);
  Serial.begin(115200);
  Serial.println("Companion Disconnected");
  //  COMP.ConnectionSleep(10);
}

void loop()
{
  CompanionConnection(COMP.Run());
  if (CompanionConnection.stateChange())
  {
    Serial.println(CompanionConnection ? "Companion Connected" : "Companion Disconnected");
  }

  {
    byte buffer[20];
    uint8_t bytes = 20;
    uint8_t retBytes = COMP.GetCommand(buffer, bytes);
    if (retBytes)
    {
      Serial.print("UDP Command: ");
      for (uint8_t x = 0; x < retBytes; x++)
        Serial.print((char)buffer[x]);
      Serial.println();
    }
  }
}
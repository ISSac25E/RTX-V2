#define RTX_SOFTWARE_VERSION {1,0,0}
#define RTX_DEVICE_NAME "TEST"
#define RTX_DEVICE_IP 2

#include "RTX_PROTOCAL.h"

void setup() {
  Serial.begin(2000000);
  Serial.println("INIT");
  pinMode(13,OUTPUT);
}

void loop() {
  uint8_t IP;
  uint8_t Bytes;
  uint8_t MSG[10];
  if (RTX_PROTOCAL.Read(IP, Bytes, MSG)) {
    digitalWrite(13,MSG[1] & B00000001);
    Serial.println("MSG RECIEVED:");
    Serial.print("IP: ");
    Serial.println(IP);
    Serial.print("MSG: ");
    for (uint8_t X = 0; X < Bytes; X++) {
      for (uint8_t Y = 0; Y < 8; Y++) Serial.print((MSG[X] & (1 << (7 - Y))) >> (7 - Y));
      //      Serial.print(MSG[X]);
      if (X < Bytes - 1) Serial.print("  ");
    }
    Serial.println();
    Serial.println();
  }

}

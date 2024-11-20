#include "ONEWIRE_RTX.h"

ONEWIRE_Master Master;
bool _DeviceConnected_Prev[8];
bool _DeviceConnected[8];
byte IP[] = {0, 1, 2, 3, 4, 5, 6, 7};

void setup() {
  Master.SetPin(8);
  Master.SetDevices(IP, sizeof(IP));
  Serial.begin(115200);
}

void loop() {
  Master.Run();
  for (uint8_t X = 0; X < 8; X++) {
    if (Master.DeviceConnected(X) != _DeviceConnected[X]) {
      Serial.print("IP ");
      Serial.println(IP[X]);
      if (Master.DeviceConnected(X)) {
        Serial.println("Connected");
      }
      else {
        Serial.println("Disconnected");
      }
    }
    _DeviceConnected[X] = Master.DeviceConnected(X);
  }
}

//#include "ONEWIRE_RTX.h"
//
//ONEWIRE_Slave Slave;
//
//void setup() {
//  Serial.begin(115200);
//  Slave.SetIP(0);
//  Slave.SetPin(8);
//}
//
//uint8_t MSG[10];
//uint8_t IP;
//uint8_t Bytes;
//
//void loop() {
//
////   if(Slave.Read(IP, Bytes, MSG)) {
////    if(IP == 7 && Bytes == 1) {
////      Slave.Write(0,1,MSG);
////    }
////   }
//
//  if(Serial.available()) {
//    MSG[0] = Serial.read();
//    Slave.Write(0,10,MSG);
//  }
//if(Slave.Read(IP, Bytes, MSG)) {
//  Serial.write(MSG[0]);
//}
//}

#include "ONEWIRE_RTX.h"

ONEWIRE_Master Master;
bool _DeviceConnected_Prev[8];
bool _DeviceConnected[8];
byte IP[] = {0, 1, 2, 3, 4, 5};

void setup() {
  Master.SetPin(8);
  Master.SetDevices(IP, sizeof(IP));
  Serial.begin(2000000);
  Serial.println("RTX SERIAL-DEBUG INIT");
}

void loop() {
  uint8_t RunStatus[sizeof(IP)];
  Serial.flush();
  Master.Run(RunStatus);
  bool WriteCMD = false;
  bool ConnCMD = false;
  for (uint8_t X = 0; X < sizeof(IP) && !WriteCMD; X++) {
    if (RunStatus[X] != 0) WriteCMD = true;
  }
  for (uint8_t X = 0; X < sizeof(IP) && !ConnCMD; X++) {
    if (Master.DeviceConnected(X) != _DeviceConnected[X]) ConnCMD = true;
  }
  if (ConnCMD) {
    Serial.print("CON: ");
    for (uint8_t X = 0; X < sizeof(IP); X++) {
        Serial.print(Master.DeviceConnected(X));
        if (X < sizeof(IP) - 1) Serial.print(F(","));
      }
    for (uint8_t X = 0; X < sizeof(IP); X++) {
      if (Master.DeviceConnected(X) != _DeviceConnected[X]) {
        Serial.print(": ");
        Serial.print("IP ");
        Serial.print(IP[X]);
        if (Master.DeviceConnected(X)) {
          Serial.print(" Connected, ");
        }
        else {
          Serial.print(" Disconnected, ");
        }
        _DeviceConnected[X] = Master.DeviceConnected(X);
      }
    }
    Serial.println();
  }
  if (WriteCMD) {
    Serial.print("RUN-CMD: ");
    for (uint8_t X = 0; X < sizeof(IP); X++) {
      Serial.print(RunStatus[X]);
      if (X < sizeof(IP) - 1) Serial.print(F(","));
    }
    Serial.println();
  }
  if(WriteCMD || ConnCMD) Serial.println();
  if(Serial.available()) {
    while(Serial.available()) Serial.read();
    _SlaveBuffer.SerialReadAvail();
//    Master._SlaveBuffer.SerialWrite();
//    _SlaveBuffer.SerialBuffer();
//    Serial.println('k');
  }
  //  for (uint8_t X = 0; X < 8; X++) {
  //    if (Master.DeviceConnected(X) != _DeviceConnected[X]) {
  //      Serial.print("IP ");
  //      Serial.println(IP[X]);
  //      if (Master.DeviceConnected(X)) {
  //        Serial.println("Connected");
  //      }
  //      else {
  //        Serial.println("Disconnected");
  //      }
  //    }
  //    _DeviceConnected[X] = Master.DeviceConnected(X);
  //  }
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

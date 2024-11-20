#define ONEWIRE_DATA_PIN 8
#define ONEWIRE_SLAVE_DEV_CNT 6
#define ONEWIRE_SLAVE_DEV_IP {0,1,2,3,4,5}

#include "ONEWIRE_MASTER.h"

ONEWIRE_MASTER MASTER;
DEBUG DEBUG_PIN(13);

void setup() {
  Serial.begin(2000000);
  Serial.println("INIT");
  DEBUG_PIN.PinSet(HIGH);
}

uint8_t IP[] = ONEWIRE_SLAVE_DEV_IP;
bool _DeviceConnected_Prev[8];
bool _DeviceConnected[8];
uint8_t PrevRunStatus[ONEWIRE_SLAVE_DEV_CNT];

void loop() {
  uint8_t RunStatus[ONEWIRE_SLAVE_DEV_CNT];
  Serial.flush();
  MASTER.Run(RunStatus);
  bool WriteCMD = false;
  bool ConnCMD = false;
  for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT && !WriteCMD; X++) {
    if (RunStatus[X] != 0) WriteCMD = true;
  }
  for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT && !ConnCMD; X++) {
    if (MASTER.DeviceConnected(X) != _DeviceConnected[X]) ConnCMD = true;
  }
  if (ConnCMD) {
    Serial.print("CON: ");
    for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT; X++) {
        Serial.print(MASTER.DeviceConnected(X));
        if (X < sizeof(IP) - 1) Serial.print(F(","));
      }
    for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT; X++) {
      if (MASTER.DeviceConnected(X) != _DeviceConnected[X]) {
        DEBUG_PIN.PinToggle();
        Serial.print(": ");
        Serial.print("IP ");
        Serial.print(IP[X]);
        if (MASTER.DeviceConnected(X)) {
          Serial.print(" Connected, ");
        }
        else {
          Serial.print(" Disconnected, ");
        }
        _DeviceConnected[X] = MASTER.DeviceConnected(X);
      }
    }
    Serial.println();
  }
  if (WriteCMD) {
    Serial.print("RUN-CMD: ");
    for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT; X++) {
      Serial.print(RunStatus[X]);
      if (X < ONEWIRE_SLAVE_DEV_CNT - 1) Serial.print(F(","));
    }
    Serial.println();
  }
  if(WriteCMD || ConnCMD) Serial.println();
}

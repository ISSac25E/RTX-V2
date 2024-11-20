#define ONEWIRE_DATA_PIN 8
#define ONEWIRE_SLAVE_DEV_CNT 6
#define ONEWIRE_SLAVE_DEV_IP {0,1,2,3,4,5}

#include "ONEWIRE_MASTER.h"

ONEWIRE_MASTER MASTER;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  MASTER.Run();
}

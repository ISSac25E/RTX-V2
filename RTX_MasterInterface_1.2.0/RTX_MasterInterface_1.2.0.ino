#define RTX_MAX_PACKET_HOLD 3

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\FSCC\RTX Systems\RTX V2\RTX_MASTER_INTERFACE\RTX_MasterInterface_1.2.0\include.h"

ONEWIRE_Master Master;
byte IP[] = {0, 1, 2, 3, 4, 5};

void setup()
{
  Master.SetPin(8);
  Master.SetDevices(IP, sizeof(IP));
}

void loop()
{
  Master.Run();
}
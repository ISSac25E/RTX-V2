#include <CORE.h>
#include "Projects/RTX/ADVANCED_SERIAL/ADVANCED_SERIAL_1.0.1.h"

AdvancedSerial AdvSerial;

uint8_t SerialBuffer[48];
uint8_t BytesAvail;

void setup()
{
  AdvSerial.Init(115200);
  pinMode(13, OUTPUT);
}

void loop()
{

  if (AdvSerial.Run(SerialBuffer, BytesAvail))
  {
    if (SerialBuffer[1] == 'N')
    {
      digitalWrite(13, HIGH);
    }
    else
    {
      digitalWrite(13, LOW);
    }
  }

  SerialBuffer[0] = 'M';

  if (digitalRead(13))
  {
    SerialBuffer[1] = 'N';
  }
  else
  {
    SerialBuffer[1] = 'F';
  }

  {
    static uint16_t SerialCount = 0;
    SerialBuffer[2] = (SerialCount / 10000 + '0');
    SerialBuffer[3] = ((SerialCount % 10000) / 1000 + '0');
    SerialBuffer[4] = (((SerialCount % 10000) % 1000) / 100 + '0');
    SerialBuffer[5] = ((((SerialCount % 10000) % 1000) % 100) / 10 + '0');
    SerialBuffer[6] = (((((SerialCount % 10000) % 1000) % 100) % 10) + '0');
    SerialCount++;
  }
  // AdvSerial.Write(SerialBuffer, 2);
  AdvSerial.Write(SerialBuffer, 7);

  delay(5);
}
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
    char TestMSG[] = "HelloWorld";
    if (CompChar(SerialBuffer, 1, TestMSG, 10))
      digitalWrite(13, HIGH);
  }
}

bool CompChar(char *Buffer, uint8_t Index, char *CompBuff, uint8_t Bytes)
{
  bool CompResult = true;
  uint8_t CompBuffInd = 0;
  while (Buffer[Index] == ' ')
    Index++;
  while (Buffer[Index] != ' ' && CompResult)
  {
    if (CompBuffInd >= Bytes || Buffer[Index] != CompBuff[CompBuffInd])
      CompResult = false;
    Index++;
    CompBuffInd++;
  }
  return CompResult;
}

int ParseInt(char *Buffer, uint8_t Index)
{
  int Result = -1;
  if (Buffer[Index] >= '0' && Buffer[Index] <= '9')
  {
    Result = 0;
    while (Buffer[Index] >= '0' && Buffer[Index] <= '9')
    {
      Result *= 10;
      Result += Buffer[Index] - '0';
      Index++;
    }
  }
  return Result;
}
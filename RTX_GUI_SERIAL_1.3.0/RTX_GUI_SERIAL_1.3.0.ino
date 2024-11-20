#define RTX_DEVICE_NAME "GUI_S"
#define RTX_SOFTWARE_VERSION \
  {                          \
    1, 3, 0                  \
  }
#define RTX_DEVICE_IP 0
#define RTX_REQ_MACROS
#define RTX_NCLR_DEFS
#define RTX_DEBUG_SECTORS 4
#define RTX_EEPROM_SECTORS 4

#define EEPROM_WRITE_LED_EN
#define EEPROM_WRITE_LED_PIN 13

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\FSCC\RTX Systems\RTX V2\RTX_GUI_SERIAL\RTX_GUI_SERIAL_1.3.0\include.h"

#define SERIAL_BAUD 115200

void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  char softVer_char[12];
  {
    uint8_t softVer[3] = RTX_SOFTWARE_VERSION;
    getVersion(softVer, softVer_char);
  }
  Serial.print("RTX GUI SERIAL ");
  Serial.println(softVer_char);
  Serial.println();
}

void loop()
{
}

void RTX_delay(uint32_t delay)
{
  uint32_t tim = millis();
  while(millis() - tim < delay)
    RTX_PROTOCAL.Run();
}

/*
  expect standard format for num input
  three number no more
  if the first numbers is '-1', then it will be redacted
  "N/A" if all numbers are invalid

  largest possible version length : "vvv.vvv.vvv"
  12 byte char array required!
*/
void getVersion(uint8_t *softVer_num, char *softVer_char)
{
  bool naCode = true;
  uint8_t charIndex = 0;

  for (uint8_t x = 0; x < 3; x++)
  {
    if ((int8_t)softVer_num[x] != -1)
      naCode = false;

    if ((int8_t)softVer_num[x] != -1)
    {
      if (softVer_num[x] > 99)
        softVer_char[charIndex++] = (softVer_num[x] / 100) + '0';
      if (softVer_num[x] > 9)
        softVer_char[charIndex++] = ((softVer_num[x] / 10) % 10) + '0';
      softVer_char[charIndex++] = (softVer_num[x] % 10) + '0';
      if (x < 2)
        softVer_char[charIndex++] = '.';
    }
  }
  if (naCode)
  {
    softVer_char[0] = 'N';
    softVer_char[1] = '/';
    softVer_char[2] = 'A';
    softVer_char[3] = 0;
  }
  else
    softVer_char[charIndex] = 0;
}
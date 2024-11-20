#include "ONEWIRE_RTX.h"
#include "ATEM_UIP.h"
#include "COMPANION_UIP.h"
#include "RTX_EEPROM.h"

ONEWIRE_Slave RTX;
ATEM_UIP ATEM;
COMP_UIP COMP;

#define RTX_EEPROM_SECTORS 8
#define RTX_DEBUG_SECTORS 8

#define RTX_DEVICE_NAME "ETHER"
#define RTX_SOFTWARE_VERSION = "1 0 0";

bool RTX_EEPROM_ERROR;

uint8_t RTX_EEPROM_BUFFER[RTX_EEPROM_SECTORS * 8];
uint8_t RTX_DEBUG_BUFFER[RTX_DEBUG_SECTORS * 8];


void setup() {
  RTX.SetIP(1);
  RTX.SetPin(8);
  RTX_EEPROM_ERROR = !RTX_EEPROM.Read(RTX_EEPROM_BUFFER, RTX_EEPROM_SECTORS);
  if(!RTX_EEPROM_ERROR) {
    
  }
}

void loop() {
  RTX_PROTOCAL_RUN();

}

void RTX_PROTOCAL_RUN() {
  uint8_t RX_IP;
  uint8_t RX_BYTES;
  uint8_t RX_MSG[10];
  if (RTX.Read(RX_IP, RX_BYTES, RX_MSG)) {
    if (RX_BYTES) {
      switch (RX_MSG[0] & B00000011) {
        case 0:
          //System Comand MSG
          if (!(RX_MSG[0] & B00100000)) {
          switch ((RX_MSG[0] & B00011100) >> 2) {
              case 0:
                //Ping
                if (RX_BYTES == 1) {
                  uint8_t TX_MSG[1];
                  TX_MSG[0] = B00100000;
                  RTX.Write(RX_IP, 1, TX_MSG);
                }
                break;
              case 1:
                //Device Name
                if (RX_BYTES == 1) {
                  uint8_t NameLength;
                  if (sizeof(RTX_DEVICE_NAME) - 1 <= 5) NameLength = sizeof(RTX_DEVICE_NAME) - 1;
                  else NameLength = 5;
                  uint8_t TX_MSG[NameLength + 1];
                  TX_MSG[0] = B00100100;
                  for (uint8_t X = 0; X < NameLength; X++) TX_MSG[X + 1] = RTX_DEVICE_NAME[X];
                  RTX.Write(RX_IP, NameLength + 1, TX_MSG);
                }
                break;
              case 2:
                //Software Version
                if (RX_BYTES == 1) {
                  uint8_t TX_MSG[4];
                  TX_MSG[0] = B00101000;
                  
                  for (uint8_t X = 0; X < 3; X++) TX_MSG[X + 1] = RTX_SOFTWARE_VERSION[X];
                  RTX.Write(RX_IP, 4, TX_MSG);
                }
                break;
              case 3:
                //Software Date
                if (RX_BYTES == 1) {
                  uint8_t TX_MSG[4];
                  TX_MSG[0] = B00101100;
                  char Month[] = {__DATE__[0], __DATE__[1], __DATE__[2], '\0'};
                  if (!strcmp(Month, "Jan"))
                    TX_MSG[2] = 1;
                  else if (!strcmp(Month, "Feb"))
                    TX_MSG[2] = 2;
                  else if (!strcmp(Month, "Mar"))
                    TX_MSG[2] = 3;
                  else if (!strcmp(Month, "Apr"))
                    TX_MSG[2] = 4;
                  else if (!strcmp(Month, "May"))
                    TX_MSG[2] = 5;
                  else if (!strcmp(Month, "Jun"))
                    TX_MSG[2] = 6;
                  else if (!strcmp(Month, "Jul"))
                    TX_MSG[2] = 7;
                  else if (!strcmp(Month, "Aug"))
                    TX_MSG[2] = 8;
                  else if (!strcmp(Month, "Sep"))
                    TX_MSG[2] = 9;
                  else if (!strcmp(Month, "Oct"))
                    TX_MSG[2] = 10;
                  else if (!strcmp(Month, "Nov"))
                    TX_MSG[2] = 11;
                  else if (!strcmp(Month, "Dec"))
                    TX_MSG[2] = 12;
                  else TX_MSG[2] = 0;

                  TX_MSG[1] = (__DATE__[5] - '0');
                  TX_MSG[1] += (__DATE__[4] - '0') * 10;
                  TX_MSG[3] = (__DATE__[10] - '0');
                  TX_MSG[3] += (__DATE__[9] - '0') * 10;
                  
                  RTX.Write(RX_IP, 4, TX_MSG);
                }
                break;
              case 4:
                //Software Time
                if (RX_BYTES == 1) {
                  uint8_t TX_MSG[4];
                  TX_MSG[0] = B00110000;

                  TX_MSG[1] = (__TIME__[1] - '0');
                  TX_MSG[1] += (__TIME__[0] - '0') * 10;
                  TX_MSG[2] = (__TIME__[4] - '0');
                  TX_MSG[2] += (__TIME__[3] - '0') * 10;
                  TX_MSG[3] = (__TIME__[7] - '0');
                  TX_MSG[3] += (__TIME__[6] - '0') * 10;

                  RTX.Write(RX_IP, 4, TX_MSG);
                }
                break;
            }
          }
          break;
        case 1:
            //EEPROM Comand MSG
            if (!(RX_MSG[0] & B00100000)) {
            switch ((RX_MSG[0] & B00011100) >> 2) {
                case 0:
                  //Buffer Read
                  if (RX_BYTES == 2) {
                    if (!(RX_MSG[0] & B01000000)) {
                      //EEPROM Buffer
                      if (RX_MSG[1] < RTX_EEPROM_SECTORS * 8) {
                        uint8_t TX_MSG[3];
                        TX_MSG[0] = B00100001;
                        TX_MSG[1] = RX_MSG[1];
                        TX_MSG[2] = RTX_EEPROM_BUFFER[RX_MSG[1]];
                        RTX.Write(RX_IP, 3, TX_MSG);
                      }
                      else {
                        //ERROR: Out Of Range
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B10100001;
                        TX_MSG[1] = RX_MSG[1];
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                    }
                    else {
                      //DEBUG Buffer
                      if (RX_MSG[1] < RTX_DEBUG_SECTORS * 8) {
                        uint8_t TX_MSG[3];
                        TX_MSG[0] = B01100001;
                        TX_MSG[1] = RX_MSG[1];
                        TX_MSG[2] = RTX_DEBUG_BUFFER[RX_MSG[1]];
                        RTX.Write(RX_IP, 3, TX_MSG);
                      }
                      else {
                        //ERROR: Out Of Range
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B11100001;
                        TX_MSG[1] = RX_MSG[1];
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                    }
                  }
                  break;
                case 1:
                  //Buffer Write
                  if (RX_BYTES == 3) {
                    if (!(RX_MSG[0] & B01000000)) {
                      //EEPROM Buffer
                      if (RX_MSG[1] < RTX_EEPROM_SECTORS * 8) {
                        RTX_EEPROM_BUFFER[RX_MSG[1]] = RX_MSG[2];
                        uint8_t TX_MSG[3];
                        TX_MSG[0] = B00100101;
                        TX_MSG[1] = RX_MSG[1];
                        TX_MSG[2] = RTX_EEPROM_BUFFER[RX_MSG[1]];
                        RTX.Write(RX_IP, 3, TX_MSG);
                      }
                      else {
                        //ERROR: Out Of Range
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B10100101;
                        TX_MSG[1] = RX_MSG[1];
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                    }
                    else {
                      //DEBUG Buffer
                      if (RX_MSG[1] < RTX_DEBUG_SECTORS * 8) {
                        RTX_DEBUG_BUFFER[RX_MSG[1]] = RX_MSG[2];
                        uint8_t TX_MSG[3];
                        TX_MSG[0] = B01100101;
                        TX_MSG[1] = RX_MSG[1];
                        TX_MSG[2] = RTX_DEBUG_BUFFER[RX_MSG[1]];
                        RTX.Write(RX_IP, 3, TX_MSG);
                      }
                      else {
                        //ERROR: Out Of Range
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B11100101;
                        TX_MSG[1] = RX_MSG[1];
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                    }
                  }
                  break;
                case 2:
                  //EEPROM Load
                  if (RX_BYTES == 1) {
                    RTX_EEPROM_ERROR = !RTX_EEPROM.Read(RTX_EEPROM_BUFFER, RTX_EEPROM_SECTORS);
                    uint8_t TX_MSG[1];
                    TX_MSG[0] = B00101001;
                    if (RTX_EEPROM_ERROR) TX_MSG[0] |= B10000000;
                    RTX.Write(RX_IP, 1, TX_MSG);
                  }
                  break;
                case 3:
                  //EEPROM Burn
                  if (RX_BYTES == 1) {
                    RTX_EEPROM.Write(RTX_EEPROM_BUFFER, RTX_EEPROM_SECTORS);
                    uint8_t TX_MSG[1];
                    TX_MSG[0] = B00101101;
                    RTX.Write(RX_IP, 1, TX_MSG);
                  }
                  break;
                case 4:
                  //EEPROM Status
                  if (RX_BYTES == 1) {
                    uint8_t TX_MSG[1];
                    TX_MSG[0] = B00110001;
                    if (RTX_EEPROM.WriteRun()) TX_MSG[0] |= B01000000;
                    if (RTX_EEPROM_ERROR) TX_MSG[0] |= B10000000;
                    RTX.Write(RX_IP, 1, TX_MSG);
                  }
                  break;
                case 5:
                  //EEPROM Error Reset
                  if (RX_BYTES == 1) {
                    uint8_t TX_MSG[1];
                    TX_MSG[0] = B00110101;
                    if (RX_MSG[0] & B10000000) {
                      RTX_EEPROM_ERROR = true;
                      TX_MSG[0] |= B10000000;
                    }
                    else RTX_EEPROM_ERROR = false;
                    RTX.Write(RX_IP, 1, TX_MSG);
                  }
                  break;
                case 6:
                  //Buffer Total Bytes
                  if (RX_BYTES == 1) {
                    if (!(RX_MSG[0] & B01000000)) {
                      //EEPROM Buffer Total Bytes
                      if (RTX_EEPROM_SECTORS) {
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B00111001;
                        TX_MSG[1] = (RTX_EEPROM_SECTORS * 8) - 1;
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                      else {
                        uint8_t TX_MSG[1];
                        TX_MSG[0] = B10111001;
                        RTX.Write(RX_IP, 1, TX_MSG);
                      }
                    }
                    else {
                      //DEBUG Buffer Total Bytes
                      if (RTX_DEBUG_SECTORS) {
                        uint8_t TX_MSG[2];
                        TX_MSG[0] = B01111001;
                        TX_MSG[1] = (RTX_DEBUG_SECTORS * 8) - 1;
                        RTX.Write(RX_IP, 2, TX_MSG);
                      }
                      else {
                        uint8_t TX_MSG[1];
                        TX_MSG[0] = B11111001;
                        RTX.Write(RX_IP, 1, TX_MSG);
                      }
                    }
                  }
                  break;
                case 7:
                  //EEPROM Write Count
                  if (RX_BYTES == 1) {
                    uint8_t TX_MSG[5];
                    TX_MSG[0] = B00111101;
                    for (uint8_t X = 0; X < 4; X++) TX_MSG[4 - X] = (RTX_EEPROM.WriteCount() >> (8 * X));
                    RTX.Write(RX_IP, 5, TX_MSG);
                  }
                  break;
              }
            }
          break;
        case 2:
            //General MSG Command
            break;
          }
    }
  }
}







/* Test:


#define RTX_SOFTWARE_VERSION "124056007"

void setup() {
  Serial.begin(115200);
  uint8_t Version[3] = {0, 0, 0};
  for (uint8_t X = 0; X < 3; X++) {
      Version[X] = (RTX_SOFTWARE_VERSION[0 + (X * 3)] - '0') * 100;
      Version[X] += (RTX_SOFTWARE_VERSION[1 + (X * 3)] - '0') * 10;
      Version[X] += (RTX_SOFTWARE_VERSION[2 + (X * 3)] - '0') * 1;
  }
  for(uint8_t X = 0; X < 3; X++) Serial.println(Version[X]);

}

void loop() {
  // put your main code here, to run repeatedly:

}

*/
 */

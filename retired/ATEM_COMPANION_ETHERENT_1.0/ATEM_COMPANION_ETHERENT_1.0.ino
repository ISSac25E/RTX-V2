#include "ONEWIRE_RTX.h"
#include "ATEM_UIP.h"
#include "COMPANION_UIP.h"
#include "RTX_EEPROM.h"

ONEWIRE_Slave RTX;
ATEM_UIP ATEM;
COMP_UIP COMP;

#define EEPROM_SECTORS 8
#define DEBUG_SECTORS 8

#define RTX_NAME "ETHER"
const byte RTX_VERSION[3] = {1, 0, 0};

bool EEPROM_ERROR;

uint8_t EEPROM_Buffer[EEPROM_SECTORS * 8];
uint8_t DEBUG_Buffer[DEBUG_SECTORS * 8];


uint8_t RX_IP;
uint8_t RX_Bytes;
uint8_t RX_MSG[10];

void setup() {
  RTX.SetIP(1);
  RTX.SetPin(8);
  EEPROM_ERROR = !RTX_EEPROM.Read(ConfigBuffer, EEPROM_SECTORS)
}

void loop() {
  RTX_LOOP();
  if (EEPROM_ERROR) {
    //If EEPROM Error, You can blink LED to Warn
  }
}

void RTX_LOOP() {
  if (RTX.Read(SenderIP, Bytes, MSG)) {
    if (Bytes) {
      if ((MSG[0] & B00000011) == 0) {
        //System Comand MSG
        if (!(MSG[0] & B00100000)) {
          switch ((MSG[0] & B00011100) >> 2) {
            case 0:
              uint8_t TX_MSG[1];
              TX_MSG[0] = B00100000;
              RTX.Write(RX_IP, 1, TX_MSG);
              //Ping
              break;
            case 1:
              uint8_t NameLength;
              if (sizeof(RTX_NAME) - 1 <= 5) NameLength = sizeof(RTX_NAME) - 1;
              else NameLength = 5;
              uint8_t TX_MSG[NameLength + 1];
              TX_MSG[0] = B00100100;
              for (uint8_t X = 0; X < NameLength; X++) TX_MSG[X + 1] = RTX_NAME[X];
              RTX.Write(RX_IP, NameLength + 1, TX_MSG);
              //Name Abreviated
              break;
            case 2:
              uint8_t TX_MSG[4];
              TX_MSG[0] = B00101000;
              for (uint8_t X = 0; X < 3; X++) TX_MSG[X + 1] = RTX_VERSION[X];
              RTX.Write(RX_IP, 4, TX_MSG);
              //Version
              break;
            case 3:
              //TODO: Send Actuall Date:
              uint8_t TX_MSG[4];
              TX_MSG[0] = B00101100;
              for (uint8_t X = 0; X < 3; X++) TX_MSG[1 + X] = 0;
              RTX.Write(RX_IP, 4, TX_MSG);
              //Date Updated
              break;
            case 4:
              //TODO: Send Actuall Time:
              uint8_t TX_MSG[4];
              TX_MSG[0] = B00110000;
              for (uint8_t X = 0; X < 3; X++) TX_MSG[1 + X] = 0;
              RTX.Write(RX_IP, 4, TX_MSG);
              //Time Updated
              break;
          }
        }
      }
      else if ((MSG[0] & B00000011) == 1) {
        //EEPROM Comand MSG
        if (!(MSG[0] & B00100000)) {
          switch ((MSG[0] & B00011100) >> 2) {
            case 0:
              //Read
              if (Bytes == 2) {
                if (!(MSG[0] & B01000000)) {
                  //EEPROM Buffer
                  if (MSG[1] < EEPROM_SECTORS * 8) {
                    uint8_t TX_MSG[3];
                    TX_MSG[0] = B00100001;
                    TX_MSG[1] = MSG[1];
                    TX_MSG[2] = EEPROM_Buffer[TX_MSG[1]];
                    RTX.Write(RX_IP, 3, TX_MSG);
                  }
                  else {
                    //ERROR
                    uint8_t TX_MSG[2];
                    TX_MSG[0] = B10100001;
                    TX_MSG[1] = MSG[1];
                    RTX.Write(RX_IP, 2, TX_MSG);
                  }
                }
                else {
                  //DEBUG Buffer
                  TX_MSG[0] = B01100001
                  if (MSG[1] < DEBUG_SECTORS * 8) {
                    uint8_t TX_MSG[3];
                    TX_MSG[0] = B01100001;
                    TX_MSG[1] = MSG[1];
                    TX_MSG[2] = DEBUG_Buffer[TX_MSG[1]];
                    RTX.Write(RX_IP, 3, TX_MSG);
                  }
                  else {
                    //ERROR
                    uint8_t TX_MSG[2];
                    TX_MSG[0] = B11100001;
                    TX_MSG[1] = MSG[1];
                    RTX.Write(RX_IP, 2, TX_MSG);
                  }
                }
              }
              break;
            case 1:
              //Write
              if (Bytes == 3) {
                if (!(MSG[0] & B01000000)) {
                  //EEPROM Buffer
                  if (MSG[1] < EEPROM_SECTORS * 8) {
                    uint8_t TX_MSG[3];
                    EEPROM_Buffer[MSG[1]] = MSG[2];
                    TX_MSG[0] = B00100101;
                    TX_MSG[1] = MSG[1];
                    TX_MSG[2] = EEPROM_Buffer[TX_MSG[1]];
                    RTX.Write(RX_IP, 3, TX_MSG);
                  }
                  else {
                    //ERROR
                    uint8_t TX_MSG[2];
                    TX_MSG[0] = B10100101;
                    TX_MSG[1] = MSG[1];
                    RTX.Write(RX_IP, 2, TX_MSG);
                  }
                }
                else {
                  //DEBUG Buffer
                  TX_MSG[0] = B01100001
                  if (MSG[1] < DEBUG_SECTORS * 8) {
                    uint8_t TX_MSG[3];
                    DEBUG_Buffer[MSG[1]] = MSG[2]; 
                    TX_MSG[0] = B01100101;
                    TX_MSG[1] = MSG[1];
                    TX_MSG[2] = DEBUG_Buffer[TX_MSG[1]];
                    RTX.Write(RX_IP, 3, TX_MSG);
                  }
                  else {
                    //ERROR
                    uint8_t TX_MSG[2];
                    TX_MSG[0] = B11100101;
                    TX_MSG[1] = MSG[1];
                    RTX.Write(RX_IP, 2, TX_MSG);
                  }
                }
              }
              break;
            case 2:
              //Load
              if(Bytes == 2) {
                
              }
              break;
            case 3:
              //Burn
              break;
            case 4:
              //Status
              break;
            case 5:
              //Error
              break;
            case 6:
              //TotalBytes
              break;
          }
        }
      }
      else if ((MSG[0] & B00000011) == 2) {
        //General MSG Command
      }
    }
  }
}

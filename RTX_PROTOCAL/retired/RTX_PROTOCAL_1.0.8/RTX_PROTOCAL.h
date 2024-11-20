//RTX_PROTOCAL REV 1.0.8
//.h
#ifndef RTX_PROTOCAL_h
#define RTX_PROTOCAL_h

#include "Arduino.h"
#include "ONEWIRE_RTX.h"
#include "RTX_EEPROM.h"

#ifndef RTX_EEPROM_SECTORS
#define RTX_EEPROM_SECTORS 0
#endif
#ifndef RTX_DEBUG_SECTORS
#define RTX_DEBUG_SECTORS 0
#endif

#ifndef RTX_DEVICE_NAME
#define RTX_DEVICE_NAME ""
#endif
#ifndef RTX_SOFTWARE_VERSION
#define RTX_SOFTWARE_VERSION {0,0,0}
#endif

#ifndef RTX_DEVICE_IP
#define RTX_DEVICE_IP 0
#endif
#ifndef RTX_DEVICE_PIN
#define RTX_DEVICE_PIN 8
#endif

#ifdef RTX_REQ_MACROS
#define RTX_REQ_PING {B00000000}
#define RTX_REQ_PING_LEN 1

#define RTX_REQ_DEV_NAME {B00000100}
#define RTX_REQ_DEV_NAME_LEN 1

#define RTX_REQ_SOFT_VER {B00001000}
#define RTX_REQ_SOFT_VER_LEN 1

#define RTX_REQ_SOFT_DATE {B00001100}
#define RTX_REQ_SOFT_DATE_LEN 1

#define RTX_REQ_SOFT_TIME {B00010000}
#define RTX_REQ_SOFT_TIME_LEN 1

#define RTX_REQ_CONFIG_READ(ADD) {B00000001, ADD}
#define RTX_REQ_CONFIG_READ_LEN 2

#define RTX_REQ_DEBUG_READ(ADD) {B01000001, ADD}
#define RTX_REQ_DEBUG_READ_LEN 2

#define RTX_REQ_CONFIG_WRITE(ADD, VAL) {B00000101, ADD, VAL}
#define RTX_REQ_CONFIG_WRITE_LEN 3

#define RTX_REQ_DEBUG_WRITE(ADD, VAL) {B01000101, ADD, VAL}
#define RTX_REQ_DEBUG_WRITE_LEN 3

#define RTX_REQ_EEPROM_LOAD {B00001001}
#define RTX_REQ_EEPROM_LOAD_LEN 1

#define RTX_REQ_EEPROM_BURN {B00001101}
#define RTX_REQ_EEPROM_BURN_LEN 1

#define RTX_REQ_EEPROM_STATUS {B00010001}
#define RTX_REQ_EEPROM_STATUS_LEN 1

#define RTX_REQ_EEPROM_SET {B10010101}
#define RTX_REQ_EEPROM_SET_LEN 1

#define RTX_REQ_EEPROM_RESET {B00010101}
#define RTX_REQ_EEPROM_RESET_LEN 1

#define RTX_REQ_CONFIG_BYTES {B00011001}
#define RTX_REQ_CONFIG_BYTES_LEN 1

#define RTX_REQ_DEBUG_BYTES {B01011001}
#define RTX_REQ_DEBUG_BYTES_LEN 1

#define RTX_REQ_EEPROM_WRITE_COUNT {B00011101}
#define RTX_REQ_EEPROM_WRITE_COUNT_LEN 1
#endif

class RTX_PROTOCAL_class {

  public:

    RTX_PROTOCAL_class();

    bool Connected() {
      return RTX.Status(1);
    };

    //Used simply for Connecting and Tests
    bool Run() {
      uint8_t IP;
      uint8_t BYTES;
      uint8_t MSG[10];
      Read(IP, BYTES, MSG);
      return Connected();
    }

    //Returns if any messages recieved:
    bool Read(uint8_t &RX_IP, uint8_t &RX_Bytes, uint8_t *RX_MSG);

    bool Write(uint8_t RecieverIP, uint8_t ByteCount, uint8_t *DataArray) {
      for (uint8_t X = 0; X < 2; X++) {
        if (!RTX.Write(RecieverIP, ByteCount, DataArray)) {
          if (!RTX.Status(6) || RTX.Status(0)) return false;
        }
        else return true;
      }
      return false;
    };

    bool RTX_EEPROM_ERROR = false;

    bool RTX_EEPROM_WRITING = false;

    //[0]: 0 = System Comand MSG, 1 = EEPROM Comand MSG, 2 && 3 = General MSG Command, 4++ = ERROR:
    //[1]: 0 - 7 = Ping, Device Name, Buffer Read/Write, EEPROM Load, ect:
    //[2]: 0 - 1 = Request, Respond:
    uint8_t RTX_MSG_TYPE[3];

    uint8_t RTX_EEPROM_BUFFER[RTX_EEPROM_SECTORS * 8];
    uint8_t RTX_DEBUG_BUFFER[RTX_DEBUG_SECTORS * 8];

  private:
    //Creat RTX Object to Read and Write With:
    ONEWIRE_Slave RTX;
};
extern RTX_PROTOCAL_class RTX_PROTOCAL;

//.cpp
//#include "RTX_PROTOCAL.h"
//#include "Arduino.h"

RTX_PROTOCAL_class::RTX_PROTOCAL_class() {
  RTX.SetIP(RTX_DEVICE_IP);
  RTX.SetPin(RTX_DEVICE_PIN);
  RTX_EEPROM_ERROR = !RTX_EEPROM.Read(RTX_EEPROM_BUFFER, RTX_EEPROM_SECTORS);
};

bool RTX_PROTOCAL_class::Read(uint8_t &RX_IP, uint8_t &RX_BYTES, uint8_t *RX_MSG) {
  RTX_EEPROM_WRITING = RTX_EEPROM.WriteRun();
  if (RTX.Read(RX_IP, RX_BYTES, RX_MSG)) {
    if (RX_BYTES) {
      RTX_MSG_TYPE[0] = RX_MSG[0] & B00000011;
      switch (RX_MSG[0] & B00000011) {
        case 0:
          //System Comand MSG
          RTX_MSG_TYPE[1] = ((RX_MSG[0] & B00011100) >> 2);
          if (!(RX_MSG[0] & B00100000)) {
            RTX_MSG_TYPE[2] = 0;
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
                  uint8_t Version[3] = RTX_SOFTWARE_VERSION;
                  uint8_t TX_MSG[4];
                  TX_MSG[0] = B00101000;

                  for (uint8_t X = 0; X < 3; X++) TX_MSG[X + 1] = Version[X];
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

                  if (__DATE__[5] >= '0' && __DATE__[5] <= '9')
                    TX_MSG[1] = (__DATE__[5] - '0');
                  if (__DATE__[4] >= '0' && __DATE__[4] <= '9')
                    TX_MSG[1] += (__DATE__[4] - '0') * 10;
                  if (__DATE__[10] >= '0' && __DATE__[10] <= '9')
                    TX_MSG[3] = (__DATE__[10] - '0');
                  if (__DATE__[9] >= '0' && __DATE__[9] <= '9')
                    TX_MSG[3] += (__DATE__[9] - '0') * 10;

                  RTX.Write(RX_IP, 4, TX_MSG);
                }
                break;
              case 4:
                //Software Time
                if (RX_BYTES == 1) {
                  uint8_t TX_MSG[4];
                  TX_MSG[0] = B00110000;

                  if (__TIME__[1] >= '0' && __TIME__[1] <= '9')
                    TX_MSG[1] = (__TIME__[1] - '0');
                  if (__TIME__[0] >= '0' && __TIME__[0] <= '9')
                    TX_MSG[1] += (__TIME__[0] - '0') * 10;
                  if (__TIME__[4] >= '0' && __TIME__[4] <= '9')
                    TX_MSG[2] = (__TIME__[4] - '0');
                  if (__TIME__[3] >= '0' && __TIME__[3] <= '9')
                    TX_MSG[2] += (__TIME__[3] - '0') * 10;
                  if (__TIME__[7] >= '0' && __TIME__[7] <= '9')
                    TX_MSG[3] = (__TIME__[7] - '0');
                  if (__TIME__[6] >= '0' && __TIME__[6] <= '9')
                    TX_MSG[3] += (__TIME__[6] - '0') * 10;

                  RTX.Write(RX_IP, 4, TX_MSG);
                }
                break;
            }
          }
          else {
            RTX_MSG_TYPE[2] = 1;
          }
          break;
        case 1:
          //EEPROM Comand MSG
          RTX_MSG_TYPE[1] = ((RX_MSG[0] & B00011100) >> 2);
          if (!(RX_MSG[0] & B00100000)) {
            RTX_MSG_TYPE[2] = 0;
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
                  if (RTX_EEPROM_WRITING) TX_MSG[0] |= B01000000;
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
          else {
            RTX_MSG_TYPE[2] = 1;
          }
          break;
        case 2:
          //General MSG Command
          break;
      }
    }
    else {
      RTX_MSG_TYPE[0] = 4;
    }
    return true;
  }
  return false;
}

RTX_PROTOCAL_class RTX_PROTOCAL;
#ifndef RTX_NCLR_DEFS
#undef RTX_EEPROM_SECTORS
#undef RTX_DEBUG_SECTORS
#undef RTX_DEVICE_NAME
#undef RTX_SOFTWARE_VERSION
#undef RTX_DEVICE_IP
#undef RTX_DEVICE_PIN
#endif
#endif

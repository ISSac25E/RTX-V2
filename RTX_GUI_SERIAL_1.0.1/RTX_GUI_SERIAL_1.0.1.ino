#define RTX_DEVICE_NAME "GUI_S"
#define RTX_SOFTWARE_VERSION {1,0,0}
#define RTX_DEVICE_IP 0
#define RTX_REQ_MACROS

#include "RTX_PROTOCAL.h"

#define GUI_LOOP_INSERT RTX_PROTOCAL.Run();

#define RTX_RX_TIMEOUT (500L * 1000L)

uint8_t SlaveIP_Selected;

enum GUI_Mode_ENUM {
  IpPromt,
  CMD_Promt,
  SYS,
  BUFF,
  MSG
};

GUI_Mode_ENUM GUI_Mode = IpPromt;

void setup() {
  Serial.begin(115200);

}

void loop() {
  switch (GUI_Mode) {
    case IpPromt:
      GUI_Mode = GUI_IpPromt();
      break;
    case CMD_Promt:
      GUI_Mode = GUI_CMD_Promt();
      break;
    case SYS:
      GUI_Mode = GUI_SYS();
      break;
    case BUFF:
      GUI_Mode = GUI_BUFF();
      break;
    case MSG:
      GUI_Mode = GUI_MSG();
      break;

  }
}

uint8_t GUI_IpPromt() {
  Serial.print(F("ENTER SLAVE IP: "));
  while (1) {
    if (Serial.available()) {
      //Check if next byte is a number
      if (ParseNext() == 1) {
        int16_t ParseSerial_Hold = ParseInt();
        //Make sure there is no more left:
        if (ParseNext() == 0) {
          if (ParseSerial_Hold != -1 && ParseSerial_Hold < 8) {
            SlaveIP_Selected = ParseSerial_Hold;
            Serial.println(SlaveIP_Selected);
            Serial.println();
            return CMD_Promt; //GUI_CMD_Promt Next
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
    }
    GUI_LOOP_INSERT
  }
}

uint8_t GUI_CMD_Promt() {
  Serial.print(F("SELECT CMD: SYSTEM(S), CONFIG/DEBUG-BUFFERS(B), GENERAL-MESSAGE(M): "));
  while (1) {
    if (Serial.available()) {
      //Check if Next is a Char:
      if (ParseNext() == 2) {
        int16_t ParseSerial_Hold = ParseChar();
        //Make sure there is no more left:
        if (ParseNext() == 0) {
          //Make sure we do have a char:
          if (ParseSerial_Hold != -1) {
            switch (ParseSerial_Hold) {
              case 'S':
                //SYSTEM(S)
                Serial.println(F("SYSTEM"));
                Serial.println();
                return SYS;
              case 'B':
                //CONFIG/DEBUG-BUFFERS(B)
                Serial.println(F("CONFIG/DEBUG-BUFFERS"));
                Serial.println();
                return BUFF;
              case 'M':
                //GENERAL-MESSAGE(M)
                Serial.println(F("GENERAL-MESSAGE"));
                Serial.println();
                return MSG;
              case 'C':
                //CANCEL(C)
                Serial.println(F("CANCEL"));
                Serial.println();
                Serial.println();
                return IpPromt;
              case 'E':
                //CANCEL(E)
                Serial.println(F("EXIT"));
                Serial.println();
                Serial.println();
                return IpPromt;
            }
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
    }
    GUI_LOOP_INSERT
  }
}

uint8_t GUI_SYS() {
  Serial.print(F("SYSTEM CMD: PING(P), DEVICE NAME(N), SOFT VER(S), DATE(D), TIME(T): "));
  while (1) {
    if (Serial.available()) {
      //Check if Next is a Char:
      if (ParseNext() == 2) {
        int16_t ParseSerial_Hold = ParseChar();
        //Make sure there is no more left:
        if (ParseNext() == 0) {
          //Make sure we do have a char:
          if (ParseSerial_Hold != -1) {
            switch (ParseSerial_Hold) {
              case 'P':
                Serial.println(F("PING(P)"));
                Serial.println();
                Serial.print(F("REQUESTING PING FROM IP: "));
                Serial.println(SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_PING_LEN] = RTX_REQ_PING;
                  if (RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_PING_LEN, RTX_TX)) {
                    {
                      uint32_t RTX_ReadTimeOut = micros();
                      uint32_t RTX_PingDelay;
                      bool ResponseFound = false;
                      while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                        //TODO Read from RTX, wait for PING response
                        uint8_t RX_IP;
                        uint8_t RX_Bytes;
                        uint8_t RX_MSG[10];
                        if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                          if (RX_IP == SlaveIP_Selected) {
                            //Corrrect IP
                            //Make sure it is System MSG and Ping Response:
                            if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 1) {
                              RTX_PingDelay = micros() - RTX_ReadTimeOut;
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                      if (ResponseFound) {
                        //Ping MSG Verified:
                        Serial.print(F("PING RESPONSE || PING: "));
                        Serial.print(RTX_PingDelay / 1000);
                        Serial.print(".");
                        Serial.print(RTX_PingDelay % 1000);
                        Serial.println(F("ms"));
                        Serial.println();
                        Serial.println();
                      }
                      else {
                        Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                        Serial.println();
                        Serial.println();
                      }
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND "));
                    Serial.println();
                    Serial.println();
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                }
                return SYS;
              case 'N':
                Serial.println(F("DEVICE NAME(N)"));
                Serial.println();
                Serial.print(F("REQUESTING DEVICE NAME FROM IP: "));
                Serial.println(SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_DEV_NAME_LEN] = RTX_REQ_DEV_NAME;
                  if (RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEV_NAME_LEN, RTX_TX)) {
                    {
                      uint32_t RTX_ReadTimeOut = millis();
                      char DeviceName[5];
                      uint8_t DaviceNameLen;
                      bool ResponseFound = false;
                      while (millis() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                        //TODO Read from RTX, wait for PING response
                        uint8_t RX_IP;
                        uint8_t RX_Bytes;
                        uint8_t RX_MSG[10];
                        if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                          if (RX_IP == SlaveIP_Selected) {
                            //Corrrect IP
                            //Make sure it is System MSG and Ping Response:
                            if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1) {
                              DaviceNameLen = (RX_Bytes - 1);
                              for (uint8_t X = 0; X < DaviceNameLen; X++) DeviceName[X] = RX_MSG[X + 1];
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                      if (ResponseFound) {
                        //Ping MSG Verified:
                        Serial.print(F("DEVICE NAME RESPONSE || NAME: "));
                        for (uint8_t X = 0; X < DaviceNameLen; X++) Serial.print((char)DeviceName[X]);
                        Serial.println();
                        Serial.println();
                      }
                      else {
                        Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                        Serial.println();
                        Serial.println();
                      }
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND "));
                    Serial.println();
                    Serial.println();
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                }
                return SYS;
              case 'S':
                Serial.println(F("SOFTWARE VERSION(S)"));
                Serial.println();
                Serial.print(F("REQUESTING SOFTWARE VERSION FROM IP: "));
                Serial.println(SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_SOFT_VER_LEN] = RTX_REQ_SOFT_VER;
                  if (RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_VER_LEN, RTX_TX)) {
                    {
                      uint32_t RTX_ReadTimeOut = millis();
                      uint8_t DeviceSoftVer[3];
                      uint8_t DeviceSoftVerLen;
                      bool ResponseFound = false;
                      while (millis() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                        //TODO Read from RTX, wait for PING response
                        uint8_t RX_IP;
                        uint8_t RX_Bytes;
                        uint8_t RX_MSG[10];
                        if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                          if (RX_IP == SlaveIP_Selected) {
                            //Corrrect IP
                            //Make sure it is System MSG and Ping Response:
                            if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 2 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 4) {
                              DeviceSoftVerLen = (RX_Bytes - 1);
                              for (uint8_t X = 0; X < DeviceSoftVerLen; X++) DeviceSoftVer[X] = RX_MSG[X + 1];
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                      if (ResponseFound) {
                        //Ping MSG Verified:
                        Serial.print(F("DEVICE SOFTWARE VERSION RESPONSE || SOFT VER: "));
                        for (uint8_t X = 0; X < DeviceSoftVerLen; X++) {
                          Serial.print(DeviceSoftVer[X]);
                          if (X < DeviceSoftVerLen - 1) Serial.print(F("."));
                        }
                        Serial.println();
                        Serial.println();
                      }
                      else {
                        Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                        Serial.println();
                        Serial.println();
                      }
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND "));
                    Serial.println();
                    Serial.println();
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                }
                return SYS;
              case 'D':
                Serial.println(F("UPLOAD DATE(D)"));
                Serial.println();
                Serial.print(F("REQUESTING UPLOAD DATE FROM IP: "));
                Serial.println(SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_SOFT_DATE_LEN] = RTX_REQ_SOFT_DATE;
                  if (RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_DATE_LEN, RTX_TX)) {
                    {
                      uint32_t RTX_ReadTimeOut = millis();
                      uint8_t DeviceDate[3];
                      uint8_t DeviceDateLen;
                      bool ResponseFound = false;
                      while (millis() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                        //TODO Read from RTX, wait for PING response
                        uint8_t RX_IP;
                        uint8_t RX_Bytes;
                        uint8_t RX_MSG[10];
                        if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                          if (RX_IP == SlaveIP_Selected) {
                            //Corrrect IP
                            //Make sure it is System MSG and Ping Response:
                            if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 3 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 4) {
                              DeviceDateLen = (RX_Bytes - 1);
                              for (uint8_t X = 0; X < DeviceDateLen; X++) DeviceDate[X] = RX_MSG[X + 1];
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                      if (ResponseFound) {
                        //Ping MSG Verified:
                        Serial.print(F("DEVICE UPLOAD DATE RESPONSE || UPLOAD DATE(D-M-Y): "));
                        for (uint8_t X = 0; X < DeviceDateLen; X++) {
                          Serial.print(DeviceDate[X]);
                          if (X < DeviceDateLen - 1) Serial.print(F("-"));
                        }
                        Serial.println();
                        Serial.println();
                      }
                      else {
                        Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                        Serial.println();
                        Serial.println();
                      }
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND "));
                    Serial.println();
                    Serial.println();
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                }
                return SYS;
              case 'T':
                Serial.println(F("UPLOAD TIME(T)"));
                Serial.println();
                Serial.print(F("REQUESTING UPLOAD DATE FROM IP: "));
                Serial.println(SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_SOFT_TIME_LEN] = RTX_REQ_SOFT_TIME;
                  if (RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_TIME_LEN, RTX_TX)) {
                    {
                      uint32_t RTX_ReadTimeOut = millis();
                      uint8_t DeviceTime[3];
                      uint8_t DeviceTimeLen;
                      bool ResponseFound = false;
                      while (millis() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                        //TODO Read from RTX, wait for PING response
                        uint8_t RX_IP;
                        uint8_t RX_Bytes;
                        uint8_t RX_MSG[10];
                        if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                          if (RX_IP == SlaveIP_Selected) {
                            //Corrrect IP
                            //Make sure it is System MSG and Ping Response:
                            if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 4 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 4) {
                              DeviceTimeLen = (RX_Bytes - 1);
                              for (uint8_t X = 0; X < DeviceTimeLen; X++) DeviceTime[X] = RX_MSG[X + 1];
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                      if (ResponseFound) {
                        //Ping MSG Verified:
                        Serial.print(F("DEVICE UPLOAD TIME RESPONSE || UPLOAD TIME(H-M-S): "));
                        for (uint8_t X = 0; X < DeviceTimeLen; X++) {
                          Serial.print(DeviceTime[X]);
                          if (X < DeviceTimeLen - 1) Serial.print(F("-"));
                        }
                        Serial.println();
                        Serial.println();
                      }
                      else {
                        Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                        Serial.println();
                        Serial.println();
                      }
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND "));
                    Serial.println();
                    Serial.println();
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                }
                return SYS;
              case 'C':
                Serial.println(F("CANCEL(C)"));
                Serial.println();
                return CMD_Promt;
              case 'E':
                Serial.println(F("EXIT(E)"));
                Serial.println();
                Serial.println();
                return IpPromt;
            }
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
    }
    GUI_LOOP_INSERT
  }
}
uint8_t GUI_BUFF() {
  Serial.println(F("BUFFER CMD: CONFIG BUFFER R/W/TB(BC /# /#), DEBUG BUFFER R/W/TB(BD /# /#), EEPROM LOAD/BURN(EL/B),"));
  Serial.print(F("\t\tEEPROM STATUS/WRITE COUNT(ES/W), EEPROM RESET(ER): "));
  while (1) {
    if (Serial.available()) {
      //Check if Next is a Char:
      if (ParseNext() == 2) {
        int16_t ParseSerial_Hold = ParseChar();
        //Make sure there is no more left:
        switch (ParseSerial_Hold) {
          case 'B':
            if (ParseNext() == 2) {
              ParseSerial_Hold = ParseChar();
              if (ParseSerial_Hold == 'C') {
                if (ParseChar() == -1) {
                  if (ParseNext() == 1) {
                    uint8_t SerialNumber[2] = {ParseInt(),0};
                    if (ParseChar() == -1) {
                      if (ParseNext() == 1) {
                        //BC # #
                        SerialNumber[1] = ParseInt();
                      }
                    }
                    //BC #
                  }
                }
              }
              else if (ParseSerial_Hold == 'D') {

              }
            }
            break;
          case 'E':
            if (ParseNext() == 2) {
              ParseSerial_Hold = ParseChar();
              if (ParseSerial_Hold == 'L') {
                if (ParseNext() == 0) {
                  //EEPROM LOAD
                  Serial.println(F("EEPROM LOAD(EL)"));
                }
              }
              else if (ParseSerial_Hold == 'B') {
                if (ParseNext() == 0) {
                  //EEPROM BURN
                  Serial.println(F("EEPROM BURN(EB)"));
                }
              }
              else if (ParseSerial_Hold == 'S') {
                if (ParseNext() == 0) {
                  //EEPROM STATUS
                  Serial.println(F("EEPROM STATUS(ES)"));
                }
              }
              else if (ParseSerial_Hold == 'W') {
                if (ParseNext() == 0) {
                  //EEPROM WRITE COUNT
                  Serial.println(F("EEPROM WRITE COUNT(EW)"));
                }
              }
              else if (ParseSerial_Hold == 'R') {
                if (ParseNext() == 0) {
                  //EEPROM ERROR RESET
                  Serial.println(F("EEPROM ERROR RESET(ER)"));
                }
              }
            }
            else if (ParseNext() == 0) {
              //EXIT
              Serial.println(F("EXIT(E)"));
            }
            break;
          case 'C':
            if (ParseNext() == 0) {
              //Cancel
              Serial.println(F("CANCEL(C)"));
            }
            break;
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
    }
    GUI_LOOP_INSERT
  }
}
uint8_t GUI_MSG() {

}



//Returns The Int Found at the Serial Buffer. Returns -1 if no Int found:
int16_t ParseInt() {
  //Delay A little to make3 sure all incoming data made it:
  //TODO: Maybe an Unnecessary Delay? Could Cause Problems
  delay(2);

  int16_t ReturnVal = -1; //Set default as -1
  uint8_t BitCount = 0; //Keep Track of how many Number Bits we are recieving in the Serial Buffer
  uint8_t BitHold[5]; //Keep Track of all incoming numbers, max is signed 16 bit(32 some thousand) 5 digits

  while (Serial.available()) {
    //Make sure the next char is a number:
    if (Serial.peek() >= '0' && Serial.peek() <= '9') {
      //Cant go over 5 digits:
      if (BitCount < 5) {
        BitHold[BitCount] = (Serial.read() - '0');
        BitCount++;
      }
    }
    else {
      //Construct the Int and Return Value
      if (BitCount) {
        //Make Sure we got some kind of value Ready, Other wise return -1
        ReturnVal = 0;
        for (uint8_t X = 0; X < BitCount; X++) {
          uint16_t ValHold = BitHold[(BitCount - 1) - X];
          for (uint8_t Y = 0; Y < X; Y++) ValHold *= 10;
          ReturnVal += ValHold;
        }
      }
      return ReturnVal;
    }
  }
  return ReturnVal;
}

//Reads a single Char from buffer. Does not read numbers. Returns -1 if not a char or nothing available
int16_t ParseChar() {
  delay(2);
  if (Serial.peek() >= 65 && Serial.peek() <= 126) return Serial.read();
  return -1;
}

//Checks what is next availble:
uint8_t ParseNext() {
  if (Serial.peek() >= '0' && Serial.peek() <= '9') return 1; //int or Bin or HEX Available
  if (Serial.peek() >= 65 && Serial.peek() <= 126) return 2; //Char Available
  return 0; //Nothing or invalid byte available
}

#define RTX_DEVICE_NAME "GUI_S"
#define RTX_SOFTWARE_VERSION {1,2,2}
#define RTX_DEVICE_IP 0
#define RTX_REQ_MACROS
#define RTX_NCLR_DEFS
#define RTX_DEBUG_SECTORS 4
#define RTX_EEPROM_SECTORS 4

#define EEPROM_WRITE_LED_EN
#define EEPROM_WRITE_LED_PIN 13

#include "RTX_PROTOCAL.h"

#define P ); Serial.print(
#define P_ ); Serial.println(

#define GUI_LOOP_INSERT RTX_PROTOCAL.Run();

#define RTX_RX_TIMEOUT (500L * 1000L)

#define SERIAL_BAUD 115200

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
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  uint8_t SOFT_VER[3] = RTX_SOFTWARE_VERSION;
  Serial.print(F("RTX GUI SERIAL ") P SOFT_VER[0] P '.' P SOFT_VER[1] P '.' P_ SOFT_VER[2]);
  Serial.println(F("CANCEL(C), EXIT(E)"));
  Serial.print(F("SERIAL DEVICE IP SET TO ") P_ RTX_DEVICE_IP);
  Serial.println();
  uint32_t StartTimer = millis();
  while (millis() - StartTimer < 1000) RTX_PROTOCAL.Run();
}

void loop() {
  while (Serial.available()) Serial.read();
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
  Serial.println(F("SELECT IP (0-7)"));
  while (1) {
    if (Serial.available()) {
      if (ParseNext() == 1) {
        int16_t Int = ParseInt();
        if (Int > -1 && Int < 8 && ParseNext() == 0) {
          SlaveIP_Selected = Int;
          Serial.print(F("IP SELECTED: ") P_ SlaveIP_Selected);
          Serial.println();
          return CMD_Promt;
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
      Serial.println();
      return IpPromt;
    }
    GUI_LOOP_INSERT
  }
}

uint8_t GUI_CMD_Promt() {
  Serial.println(F("SELECT COMMAND MESSAGE: SYSTEM(SYS), CONFIG/DEBUG-BUFFER(BUFF), GENERAL-MESSAGE(MSG)"));
  while (1) {
    if (Serial.available()) {
      if (ParseNext() == 2) {
        char C[4] = {0, 0, 0, 0};
        uint8_t CharAvail = ParseChar(C, 4);
        if (ParseNext() == 0) {
          if (CharAvail == 3 && CharComp("SYS", C, 3)) {
            Serial.println(F("SYSTEM MESSAGE"));
            Serial.println();
            return SYS;
          }
          else if (CharAvail == 4 && CharComp("BUFF", C, 4)) {
            Serial.println(F("CONFIG/DEBUG-BUFFER MESSAGE"));
            Serial.println();
            return BUFF;
          }
          else if (CharAvail == 3 && CharComp("MSG", C, 3)) {
            Serial.println(F("GENERAL MESSAGE"));
            Serial.println();
            return MSG;
          }
          else if (CharAvail == 1 && CharComp("C", C, 1)) {
            Serial.println(F("CANCEL"));
            Serial.println();
            Serial.println();
            return IpPromt;
          }
          else if (CharAvail == 1 && CharComp("E", C, 1)) {
            Serial.println(F("EXIT"));
            Serial.println();
            Serial.println();
            return IpPromt;
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
      Serial.println();
      return CMD_Promt;
    }
    GUI_LOOP_INSERT
  }
}

uint8_t GUI_SYS() {
  Serial.println(F("SYSTEM COMMAND: PING DEVICE(PNG), DEVICE NAME(NM), SOFTWARE VERSION(VER), SOFTWARE DATE-TIME(DT)"));
  while (1) {
    if (Serial.available()) {
      char C[3] = {0, 0, 0};
      uint8_t CharAvail = ParseChar(C, 3);
      if (ParseNext() == 0) {
        if (CharAvail == 3 && CharComp("PNG", C, CharAvail)) {
          Serial.println(F("PING DEVICE"));
          Serial.println();
          Serial.print(F("REQUESTING PING FROM IP: ") P_ SlaveIP_Selected);
          if (RTX_PROTOCAL.Connected()) {
            uint8_t RTX_TX[RTX_REQ_PING_LEN] = RTX_REQ_PING;
            //            noInterrupts();
            Serial.flush();
           uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_PING_LEN, RTX_TX);
            //            interrupts();
            if ((WriteStat & B00000001)) {
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
                delay(500);
              }
              else {
                Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
              Serial.println();
              Serial.println();
              delay(500);
            }
          }
          else {
            Serial.print(F("ERROR: RTX NOT CONNECTED "));
            Serial.println();
            Serial.println();
            delay(500);
          }
          return SYS;
        }
        else if (CharAvail == 2 && CharComp("NM", C, CharAvail)) {
          Serial.println(F("DEVICE NAME"));
          Serial.println();
          Serial.print(F("REQUESTING DEVICE NAME FROM IP: ") P_ SlaveIP_Selected);
          if (RTX_PROTOCAL.Connected()) {
            uint8_t RTX_TX[RTX_REQ_DEV_NAME_LEN] = RTX_REQ_DEV_NAME;
            //            noInterrupts();
            Serial.flush();
           uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEV_NAME_LEN, RTX_TX);
            //            interrupts();
            if ((WriteStat & B00000001)) {
              uint32_t RTX_ReadTimeOut = micros();
              char DeviceName[5];
              uint8_t DaviceNameLen;
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
                delay(500);
              }
              else {
                Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
              Serial.println();
              Serial.println();
              delay(500);
            }
          }
          else {
            Serial.print(F("ERROR: RTX NOT CONNECTED "));
            Serial.println();
            Serial.println();
            delay(500);
          }
          return SYS;
        }
        else if (CharAvail == 3 && CharComp("VER", C, CharAvail)) {
          Serial.println(F("SOFTWARE VERSION"));
          Serial.println();
          Serial.print(F("REQUESTING SOFTWARE VERSION FROM IP: ") P_ SlaveIP_Selected);
          if (RTX_PROTOCAL.Connected()) {
            uint8_t RTX_TX[RTX_REQ_SOFT_VER_LEN] = RTX_REQ_SOFT_VER;
            //noInterrupts();
            Serial.flush();
           uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_VER_LEN, RTX_TX);
            //interrupts();
            if ((WriteStat & B00000001)) {
              uint32_t RTX_ReadTimeOut = micros();
              uint8_t DeviceSoftVer[3];
              uint8_t DeviceSoftVerLen;
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
                delay(500);
              }
              else {
                Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
              Serial.println();
              Serial.println();
              delay(500);
            }
          }
          else {
            Serial.print(F("ERROR: RTX NOT CONNECTED "));
            Serial.println();
            Serial.println();
            delay(500);
          }
          return SYS;
        }
        else if (CharAvail == 2 && CharComp("DT", C, CharAvail)) {
          Serial.println(F("SOFTWARE DATE-TIME"));
          Serial.println();
          Serial.print(F("REQUESTING SOFTWARE DATE-TIME FROM IP: ") P_ SlaveIP_Selected);
          if (RTX_PROTOCAL.Connected()) {
            uint8_t RTX_TX[RTX_REQ_SOFT_DATE_LEN] = RTX_REQ_SOFT_DATE;
            //noInterrupts();
            Serial.flush();
           uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_DATE_LEN, RTX_TX);
            //interrupts();
            if ((WriteStat & B00000001)) {
              uint32_t RTX_ReadTimeOut = micros();
              uint8_t DeviceDate[3];
              uint8_t DeviceDateLen;
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
                    if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 3 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 4) {
                      DeviceDateLen = (RX_Bytes - 1);
                      for (uint8_t X = 0; X < DeviceDateLen; X++) DeviceDate[X] = RX_MSG[X + 1];
                      ResponseFound = true;
                    }
                  }
                }
              }
              if (ResponseFound) {
                uint8_t RTX_TX[RTX_REQ_SOFT_TIME_LEN] = RTX_REQ_SOFT_TIME;
                ResponseFound = false;
                //noInterrupts();
                Serial.flush();
               uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_SOFT_TIME_LEN, RTX_TX);
                //interrupts();
                if ((WriteStat & B00000001)) {
                  uint32_t RTX_ReadTimeOut = micros();
                  uint8_t DeviceTime[3];
                  uint8_t DeviceTimeLen;
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
                    Serial.print(F("DEVICE SOFTWARE DATE-TIME RESPONSE || UPLOAD DATE(D-M-Y): "));
                    for (uint8_t X = 0; X < DeviceDateLen; X++) {
                      Serial.print(DeviceDate[X]);
                      if (X < DeviceDateLen - 1) Serial.print(F("-"));
                    }
                    Serial.print(F(" UPLOAD TIME(H-M-S): "));
                    for (uint8_t X = 0; X < DeviceTimeLen; X++) {
                      Serial.print(DeviceTime[X]);
                      if (X < DeviceTimeLen - 1) Serial.print(F("-"));
                    }
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                  else {
                    Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
              }
              else {
                Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
              Serial.println();
              Serial.println();
              delay(500);
            }
          }
          else {
            Serial.print(F("ERROR: RTX NOT CONNECTED "));
            Serial.println();
            Serial.println();
            delay(500);
          }
          return SYS;
        }

        else if (CharAvail == 1 && CharComp("C", C, CharAvail)) {
          Serial.println(F("CANCEL"));
          Serial.println();
          Serial.println();
          return CMD_Promt;
        }
        else if (CharAvail == 1 && CharComp("E", C, CharAvail)) {
          Serial.println(F("EXIT"));
          Serial.println();
          Serial.println();
          return IpPromt;
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush RX
      Serial.println();
      return SYS;
    }
    GUI_LOOP_INSERT
  }
}

uint8_t GUI_BUFF() {
  Serial.println(F("SELECT BUFFER COMMAND: CONFIG/DEBUG-BUFFER TB/R/W(C/DB (/# /#)/DMP), EEPROM LOAD/BURN/STATUS/WRITE-CNT/RESET(E-L/B/S/WC/R#)"));
  while (1) {
    if (Serial.available()) {
      if (ParseNext() == 2) {
        char C[3] = {0, 0, 0};
        uint8_t CharAvail = ParseChar(C, 3);
        if (ParseNext() == 0) {
          if (CharAvail == 2 && CharComp("CB", C, CharAvail)) {
            Serial.println(F("CONFIG-BUFFER TOTAL BYTES"));
            Serial.println();
            Serial.print(F("REQUESTING CONFIG-BUFFER SIZE(BYTES) FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_CONFIG_BYTES_LEN] = RTX_REQ_CONFIG_BYTES;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_CONFIG_BYTES_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                uint16_t TotalBytes;
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
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 6 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 1)) {
                        if (!(RX_MSG[0] & B01000000)) {
                          if (!(RX_MSG[0] & B10000000)) {
                            TotalBytes = (RX_MSG[1] + 1);
                          }
                          else {
                            TotalBytes = 0;
                          }
                          ResponseFound = true;
                        }
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  //Ping MSG Verified:
                  Serial.print(F("DEVICE CONFIG-BUFFER SIZE RESPONSE || CONFIG-BUFFER SIZE(BYTES): ") P_ TotalBytes);
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 2 && CharComp("DB", C, CharAvail)) {
            Serial.println(F("DEBUG-BUFFER TOTAL BYTES"));
            Serial.println();
            Serial.print(F("REQUESTING DEBUG-BUFFER SIZE(BYTES) FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_DEBUG_BYTES_LEN] = RTX_REQ_DEBUG_BYTES;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEBUG_BYTES_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                uint16_t TotalBytes;
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
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 6 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 1)) {
                        if ((RX_MSG[0] & B01000000)) {
                          if (!(RX_MSG[0] & B10000000)) {
                            TotalBytes = (RX_MSG[1] + 1);
                          }
                          else {
                            TotalBytes = 0;
                          }
                          ResponseFound = true;
                        }
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  Serial.print(F("DEVICE CONFIG-BUFFER SIZE RESPONSE || CONFIG-BUFFER SIZE(BYTES): ") P_ TotalBytes);
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 2 && CharComp("EL", C, CharAvail)) {
            Serial.println(F("EEPROM LOAD"));
            Serial.println();
            Serial.print(F("REQUESTING EEPROM LOAD FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_EEPROM_LOAD_LEN] = RTX_REQ_EEPROM_LOAD;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_LOAD_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                bool LoadError = false;
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
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 2 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 1) {
                        if ((RX_MSG[0] & B10000000)) {
                          LoadError = true;
                        }
                        ResponseFound = true;
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  //Ping MSG Verified:
                  Serial.print(F("DEVICE EEPROM LOAD RESPONSE || "));
                  if (LoadError)
                    Serial.println(F("EEPROM LOAD ERROR"));
                  else
                    Serial.println(F("EEPROM LOAD SUCCESS"));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 2 && CharComp("EB", C, CharAvail)) {
            Serial.println(F("EEPROM BURN"));
            Serial.println();
            Serial.print(F("REQUESTING EEPROM BURN FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_EEPROM_BURN_LEN] = RTX_REQ_EEPROM_BURN;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_BURN_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                bool ResponseFound = false;
                uint8_t Sectors;
                while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                  //TODO Read from RTX, wait for PING response
                  uint8_t RX_IP;
                  uint8_t RX_Bytes;
                  uint8_t RX_MSG[10];
                  if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                    if (RX_IP == SlaveIP_Selected) {
                      //Corrrect IP
                      //Make sure it is System MSG and Ping Response:
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 3 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 2) {
                        if ((RX_MSG[0] & B10000000) && !(RX_MSG[0] & B01000000)) {
                          Sectors = RX_MSG[1];
                          ResponseFound = true;
                        }
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  //Ping MSG Verified:
                  Serial.print(F("DEVICE EEPROM BURN RESPONSE || EEPROM BURNING: ") P Sectors P_ F(" SECTORS"));
                  Serial.println();
                  Serial.print(F("0%"));
                  for (uint8_t X = 0; X < 62; X++) Serial.print(F(" "));
                  Serial.println(F("100%"));
                  Serial.flush();

                  uint8_t LoadBarCount = 0;
                  bool EEPROM_BurnDone = false;
                  uint8_t SectorRead;
                  while (!EEPROM_BurnDone) {
                    if (RTX_PROTOCAL.Connected()) {
                      uint8_t RTX_TX[RTX_REQ_EEPROM_BURN_STAT_LEN] = RTX_REQ_EEPROM_BURN_STAT;
                      //              noInterrupts();
                      Serial.flush();
                     uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_BURN_STAT_LEN, RTX_TX);
                      //              interrupts();
                      if ((WriteStat & B00000001)) {
                        uint32_t RTX_ReadTimeOut = micros();
                        ResponseFound = false;
                        while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                          //TODO Read from RTX, wait for PING response
                          uint8_t RX_IP;
                          uint8_t RX_Bytes;
                          uint8_t RX_MSG[10];
                          if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                            if (RX_IP == SlaveIP_Selected) {
                              //Corrrect IP
                              //Make sure it is System MSG and Ping Response:
                              if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 3 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 2) {
                                if ((RX_MSG[0] & B01000000)) {
                                  EEPROM_BurnDone = !(RX_MSG[0] & B10000000);
                                  if (!EEPROM_BurnDone) {
                                    SectorRead = RX_MSG[1];
                                  }
                                  else SectorRead = 0;
                                  ResponseFound = true;
                                }
                              }
                            }
                          }
                        }
                        if (ResponseFound);
                        else {
                          Serial.println();
                          Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                          Serial.println();
                          Serial.println();
                          delay(500);
                          return BUFF;
                        }
                      }
                      else {
                        Serial.println();
                        Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                        Serial.println();
                        Serial.println();
                        delay(500);
                        return BUFF;
                      }
                    }
                    else {
                      Serial.println();
                      Serial.print(F("ERROR: RTX NOT CONNECTED "));
                      Serial.println();
                      Serial.println();
                      delay(500);
                      return BUFF;
                    }
                    while (LoadBarCount < map((SectorRead & B00011111), 0, 32, 0, 68)) {
                      Serial.print(F("-"));
                      LoadBarCount++;
                    }
                  }
                  if (EEPROM_BurnDone) {
                    while (LoadBarCount < 68) {
                      Serial.print(F("-"));
                      LoadBarCount++;
                    }
                    Serial.println();
                    Serial.println(F("EEPROM DONE BURNING"));
                    Serial.println();
                    Serial.println();
                  }
                  else {
                    Serial.println();
                    Serial.print(F("ERROR: TO MANY SECTORS "));
                    Serial.println();
                    Serial.println();
                  }
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 2 && CharComp("ES", C, CharAvail)) {
            Serial.println(F("EEPROM STATUS"));
            Serial.println();
            Serial.print(F("REQUESTING EEPROM STATUS FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_EEPROM_STATUS_LEN] = RTX_REQ_EEPROM_STATUS;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_STATUS_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                bool EEPROM_Error;
                bool EEPROM_Writing;
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
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 4 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 1) {
                        EEPROM_Writing = (RX_MSG[0] & B01000000);
                        EEPROM_Error = (RX_MSG[0] & B10000000);
                        ResponseFound = true;
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  //Ping MSG Verified:
                  Serial.print(F("DEVICE EEPROM STATUS RESPONSE || EEPROM ERROR: ") P (EEPROM_Error) ? (F("TRUE")) : (F("FALSE")) P F(",   EEPROM WRITING: ") P_ (EEPROM_Writing) ? (F("TRUE")) : (F("FALSE")));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 3 && CharComp("EWC", C, CharAvail)) {
            Serial.println(F("EEPROM WRITE COUNT"));
            Serial.println();
            Serial.print(F("REQUESTING EEPROM WRITE COUNT FROM IP: ") P_ SlaveIP_Selected);
            if (RTX_PROTOCAL.Connected()) {
              uint8_t RTX_TX[RTX_REQ_EEPROM_WRITE_COUNT_LEN] = RTX_REQ_EEPROM_WRITE_COUNT;
              //              noInterrupts();
              Serial.flush();
             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_WRITE_COUNT_LEN, RTX_TX);
              //              interrupts();
              if ((WriteStat & B00000001)) {
                uint32_t RTX_ReadTimeOut = micros();
                uint32_t EEPROM_WriteCount;
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
                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 7 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 5) {
                        EEPROM_WriteCount = 0;
                        for (uint8_t X = 0; X < 4; X++) {
                          EEPROM_WriteCount |= (RX_MSG[4 - X] << (8 * X));
                        }
                        ResponseFound = true;
                      }
                    }
                  }
                }
                if (ResponseFound) {
                  //Ping MSG Verified:
                  Serial.print(F("DEVICE EEPROM WRITE COUNT RESPONSE || EEPROM WRITE COUNT: ") P_ EEPROM_WriteCount);
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                else {
                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                Serial.println();
                Serial.println();
                delay(500);
              }
            }
            else {
              Serial.print(F("ERROR: RTX NOT CONNECTED "));
              Serial.println();
              Serial.println();
              delay(500);
            }
            return BUFF;
          }
          else if (CharAvail == 1 && CharComp("C", C, CharAvail)) {
            Serial.println(F("CANCEL"));
            Serial.println();
            Serial.println();
            return CMD_Promt;
          }
          else if (CharAvail == 1 && CharComp("E", C, CharAvail)) {
            Serial.println(F("EXIT"));
            Serial.println();
            Serial.println();
            return IpPromt;
          }
        }
        else {
          if (CharAvail == 3 && CharComp("CB ", C, CharAvail)) {
            int16_t Add = ParseVal();
            if (Add > -1 && Add < 256) {
              if (ParseNext() == 0) {
                Serial.print(F("CONFIG-BUFFER READ (ADD: ") P Add P_ F(")"));
                Serial.println();
                Serial.print(F("READING CONFIG-BUFFER (ADD: ") P Add P F(") IP: ") P_ SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_CONFIG_READ_LEN] = RTX_REQ_CONFIG_READ(Add);
                  //              noInterrupts();
                  Serial.flush();
                 uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_CONFIG_READ_LEN, RTX_TX);
                  //              interrupts();
                  if ((WriteStat & B00000001)) {
                    uint32_t RTX_ReadTimeOut = micros();
                    uint8_t BufferRead[2];
                    bool BufferReadError;
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
                          if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                            if (!(RX_MSG[0] & B01000000)) {
                              BufferReadError = (RX_MSG[0] & B10000000);
                              if (BufferReadError) {
                                BufferRead[0] = RX_MSG[1];
                              }
                              else {
                                BufferRead[0] = RX_MSG[1];
                                BufferRead[1] = RX_MSG[2];
                              }
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                    }
                    if (ResponseFound) {
                      Serial.print(F("DEVICE CONFIG-BUFFER READ RESPONSE || "));
                      if (BufferReadError) {
                        Serial.print(F("ERROR: (ADD: ") P BufferRead[0] P_ F(") OUT OF RANGE"));
                      }
                      else {
                        Serial.print(F("READ: (ADD: ") P BufferRead[0] P F(", VAL: ") ); PrintBoolHexDec(BufferRead[1] P_ F(")"));
                      }
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                    else {
                      Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                return BUFF;
              }
              else {
                {
                  int16_t Val[10];
                  bool CharError = false;

                  for (uint8_t X = 0; X < 10 && !CharError; X++) {
                    if (ParseCharSingle() == ' ') {
                      Val[X] = ParseVal();
                      if (Val[X] > -1 && Val[X] < 256) {
                        if (ParseNext() == 0) {
                          Serial.print(F("CONFIG-BUFFER WRITE (ADD: ") P Add P F(", VAL: ") );
                          for (uint8_t Y = 0; Y < (X + 1); Y++) {
                            PrintBoolHexDec(Val[Y]);
                            if (Y < X) Serial.print('\t');
                          }
                          Serial.println(F(")"));
                          Serial.println();
                          Serial.print(F("WRITING CONFIG-BUFFER (ADD: ") P Add P F(", VAL: ") );
                          for (uint8_t Y = 0; Y < (X + 1); Y++) {
                            PrintBoolHexDec(Val[Y]);
                            if (Y < X) Serial.print('\t');
                          }
                          Serial.println(F(")"));
                          bool ErrorSend = false;
                          uint8_t BufferWriteResult[10];

                          for (uint8_t Y = 0; Y < (X + 1) && !ErrorSend; Y++) {

                            if (RTX_PROTOCAL.Connected()) {
                              uint8_t RTX_TX[RTX_REQ_CONFIG_WRITE_LEN] = RTX_REQ_CONFIG_WRITE(Add + Y, Val[Y]);
                              //              noInterrupts();
                              Serial.flush();
                             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_CONFIG_WRITE_LEN, RTX_TX);
                              //              interrupts();
                              if ((WriteStat & B00000001)) {
                                uint32_t RTX_ReadTimeOut = micros();
                                uint8_t BufferWrite[2];
                                bool BufferWriteError;
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
                                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                                        if (!(RX_MSG[0] & B01000000)) {
                                          BufferWriteError = (RX_MSG[0] & B10000000);
                                          if (BufferWriteError) {
                                            BufferWrite[0] = RX_MSG[1];
                                          }
                                          else {
                                            BufferWrite[0] = RX_MSG[1];
                                            BufferWrite[1] = RX_MSG[2];
                                          }
                                          ResponseFound = true;
                                        }
                                      }
                                    }
                                  }
                                }
                                if (ResponseFound) {
                                  if (BufferWriteError) {
                                    Serial.print(F("ERROR: (ADD: ") P BufferWrite[0] P_ F(") OUT OF RANGE"));
                                    Serial.println();
                                    Serial.println();
                                    delay(500);
                                    ErrorSend = true;
                                  }
                                  else {
                                    if (BufferWrite[0] == (Add + Y))
                                      BufferWriteResult[Y] = BufferWrite[1];
                                    else {
                                      Serial.print(F("ERROR: (ADD: ") P BufferWrite[0] P F(" != ") P (Add + Y) P_ F(")"));
                                      Serial.println();
                                      Serial.println();
                                      delay(500);
                                      ErrorSend = true;
                                    }
                                  }
                                }
                                else {
                                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                                  Serial.println();
                                  Serial.println();
                                  delay(500);
                                  ErrorSend = true;
                                }
                              }
                              else {
                                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                                Serial.println();
                                Serial.println();
                                delay(500);
                                ErrorSend = true;
                              }
                            }
                            else {
                              Serial.print(F("ERROR: RTX NOT CONNECTED "));
                              Serial.println();
                              Serial.println();
                              delay(500);
                              ErrorSend = true;
                            }
                          }

                          if (!ErrorSend) {
                            Serial.print(F("DEVICE CONFIG-BUFFER WRITE RESPONSE || WRITE: (ADD: ") P Add P F(", VAL: ") );
                            for (uint8_t Y = 0; Y < (X + 1); Y++) {
                               PrintBoolHexDec(BufferWriteResult[Y]);
                              if (Y < X) Serial.print('\t');
                            }
                            Serial.println(F(")"));
                            Serial.println();
                            Serial.println();
                            delay(500);
                          }

                          CharError = true;
                        }
                      }
                      else {
                        CharError = true;
                      }
                    }
                    else {
                      CharError = true;
                    }
                  }
                }
              }
            }
            else {
              CharAvail = ParseChar(C, 3);
              if (CharAvail = 3 && CharComp("DMP", C, CharAvail) && ParseNext() == 0) {
                Serial.println(F("CONFIG-BUFFER DATA DUMP"));
                Serial.print(F("DOWNLOADING CONFIG-BUFFER DATA FROM IP ") P_ SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_CONFIG_BYTES_LEN] = RTX_REQ_CONFIG_BYTES;
                  //              noInterrupts();
                  Serial.flush();
                 uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_CONFIG_BYTES_LEN, RTX_TX);
                  //              interrupts();
                  if ((WriteStat & B00000001)) {
                    uint32_t RTX_ReadTimeOut = micros();
                    uint16_t TotalBytes;
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
                          if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 6 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 1)) {
                            if (!(RX_MSG[0] & B01000000)) {
                              if (!(RX_MSG[0] & B10000000)) {
                                TotalBytes = (RX_MSG[1] + 1);
                              }
                              else {
                                TotalBytes = 0;
                              }
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                    }
                    if (ResponseFound) {
                      //Ping MSG Verified:
                      Serial.print(F("BYTES(CONFIG-BUFFER): ") P_ TotalBytes);
                      Serial.print(F("0%"));
                      for (uint8_t X = 0; X < 92; X++) Serial.print(F(" "));
                      Serial.println(F("100%"));
                      Serial.flush();
                      uint8_t LoadBarCount = 0;
                      uint8_t RTX_DataHold[TotalBytes];
                      for (uint16_t X = 0; X < TotalBytes; X++) {
                        if (Serial.available()) {
                          if (ParseNext() == 2) {
                            char C[1] = {0};
                            uint8_t CharAvail = ParseChar(C, 1);
                            if (ParseNext() == 0) {
                              if (CharAvail == 1 && CharComp("C", C, CharAvail)) {
                                Serial.println();
                                Serial.println(F("DATA DUMP CANCEL"));
                                Serial.println();
                                Serial.println();
                                return BUFF;
                              }
                            }
                          }
                          while (Serial.available()) Serial.read();
                        }
                        if (RTX_PROTOCAL.Connected()) {
                          uint8_t RTX_TX[RTX_REQ_CONFIG_READ_LEN] = RTX_REQ_CONFIG_READ(X);
                          //              noInterrupts();
                          Serial.flush();
                         uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_CONFIG_READ_LEN, RTX_TX);
                          //              interrupts();
                          if ((WriteStat & B00000001)) {
                            uint32_t RTX_ReadTimeOut = micros();
                            uint8_t BufferRead[2];
                            bool BufferReadError;
                            ResponseFound = false;
                            while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                              //TODO Read from RTX, wait for PING response
                              uint8_t RX_IP;
                              uint8_t RX_Bytes;
                              uint8_t RX_MSG[10];
                              if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                                if (RX_IP == SlaveIP_Selected) {
                                  //Corrrect IP
                                  //Make sure it is System MSG and Ping Response:
                                  if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                                    if (!(RX_MSG[0] & B01000000)) {
                                      BufferReadError = (RX_MSG[0] & B10000000);
                                      if (BufferReadError) {
                                        BufferRead[0] = RX_MSG[1];
                                      }
                                      else {
                                        BufferRead[0] = RX_MSG[1];
                                        BufferRead[1] = RX_MSG[2];
                                      }
                                      ResponseFound = true;
                                    }
                                  }
                                }
                              }
                            }
                            if (ResponseFound) {
                              if (BufferReadError) {
                                Serial.println();
                                Serial.print(F("ERROR: (ADD: ") P BufferRead[0] P_ F(") OUT OF RANGE"));
                                Serial.println();
                                Serial.println();
                                delay(500);
                                return BUFF;
                              }
                              else {
                                if (X == BufferRead[0]) {
                                  RTX_DataHold[X] = BufferRead[1];
                                }
                                else {
                                  Serial.println();
                                  Serial.print(F("ERROR: SYNCING, ") P X P F(" != ") P BufferRead[0]);
                                  Serial.println();
                                  Serial.println();
                                  delay(500);
                                  return BUFF;
                                }
                              }
                            }
                            else {
                              Serial.println();
                              Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                              Serial.println();
                              Serial.println();
                              delay(500);
                              return BUFF;
                            }
                          }
                          else {
                            Serial.println();
                            Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                            Serial.println();
                            Serial.println();
                            delay(500);
                            return BUFF;
                          }
                        }
                        else {
                          Serial.println();
                          Serial.print(F("ERROR: RTX NOT CONNECTED "));
                          Serial.println();
                          Serial.println();
                          delay(500);
                          return BUFF;
                        }
                        while (LoadBarCount < map(X, 0, (TotalBytes - 1), 0, 98)) {
                          Serial.print(F("-"));
                          LoadBarCount++;
                        }
                      }
                      while (LoadBarCount < 98) {
                        Serial.print(F("-"));
                        LoadBarCount++;
                      }
                      Serial.println();
                      Serial.println();
                      Serial.print(F("CONFIG-BUFFER(BYTES: ") P TotalBytes P_ F(")"));
                      for (uint16_t X = 0; X < TotalBytes; X++) {
                        if (!(X % 8)) {
                          Serial.println();
                          Serial.print('\t' P '\t');
                          for (uint8_t Y = 0; Y < 8 && (Y + X) < TotalBytes; Y++) {
                            Serial.print('(' P (Y + X) P ')' P '\t' P '\t' P '\t');
                          }
                          Serial.println();
                          Serial.print(X P F("-") P (X + 7) P ':' P '\t');
                          if (X < 104) Serial.print('\t');
                        }
                        for (uint8_t Y = 0; Y < 8; Y++) {
                          Serial.print(bitRead(RTX_DataHold[X], (7 - Y)));
                        }
                        Serial.print(F("(x"));
                        if (((RTX_DataHold[X] & B11110000) >> 4) > 9) {
                          Serial.write(55 + ((RTX_DataHold[X] & B11110000) >> 4));
                        }
                        else Serial.write(((RTX_DataHold[X] & B11110000) >> 4) + '0');
                        if ((RTX_DataHold[X] & B00001111) > 9) {
                          Serial.write(55 + (RTX_DataHold[X] & B00001111));
                        }
                        else Serial.write((RTX_DataHold[X] & B00001111) + '0');
                        Serial.print(F(")(") P RTX_DataHold[X] P ')' P '\t');
                      }
                      Serial.println();
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                    else {
                      Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                return BUFF;
              }
            }
          }
          else if (CharAvail == 3 && CharComp("DB ", C, CharAvail)) {
            int16_t Add = ParseVal();
            if (Add > -1 && Add < 256) {
              if (ParseNext() == 0) {
                Serial.print(F("DEBUG-BUFFER READ (ADD: ") P Add P_ F(")"));
                Serial.println();
                Serial.print(F("READING DEBUG-BUFFER (ADD: ") P Add P F(") IP: ") P_ SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_DEBUG_READ_LEN] = RTX_REQ_DEBUG_READ(Add);
                  //              noInterrupts();
                  Serial.flush();
                 uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEBUG_READ_LEN, RTX_TX);
                  //              interrupts();
                  if ((WriteStat & B00000001)) {
                    uint32_t RTX_ReadTimeOut = micros();
                    uint8_t BufferRead[2];
                    bool BufferReadError;
                    bool ResponseFound = false;
                    while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                      //TODO Read from RTX, wait for PING response
                      uint8_t RX_IP;
                      uint8_t RX_Bytes;
                      uint8_t RX_MSG[10];
                      if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                        if (RX_IP == SlaveIP_Selected) {
                          //Corrrect IP
                          if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                            if ((RX_MSG[0] & B01000000)) {
                              BufferReadError = (RX_MSG[0] & B10000000);
                              if (BufferReadError) {
                                BufferRead[0] = RX_MSG[1];
                              }
                              else {
                                BufferRead[0] = RX_MSG[1];
                                BufferRead[1] = RX_MSG[2];
                              }
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                    }
                    if (ResponseFound) {
                      Serial.print(F("DEVICE DEBUG-BUFFER READ RESPONSE || "));
                      if (BufferReadError) {
                        Serial.print(F("ERROR: (ADD: ") P BufferRead[0] P_ F(") OUT OF RANGE"));
                      }
                      else {
                        Serial.print(F("READ: (ADD: ") P BufferRead[0] P F(", VAL: ") ); PrintBoolHexDec(BufferRead[1] P_ F(")"));
                      }
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                    else {
                      Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                return BUFF;
              }
              else {
                {
                  int16_t Val[10];
                  bool CharError = false;

                  for (uint8_t X = 0; X < 10 && !CharError; X++) {
                    if (ParseCharSingle() == ' ') {
                      Val[X] = ParseVal();
                      if (Val[X] > -1 && Val[X] < 256) {
                        if (ParseNext() == 0) {
                          Serial.print(F("DEBUG-BUFFER WRITE (ADD: ") P Add P F(", VAL: ") );
                          for (uint8_t Y = 0; Y < (X + 1); Y++) {
                            PrintBoolHexDec(Val[Y]);
                            if (Y < X) Serial.print('\t');
                          }
                          Serial.println(F(")"));
                          Serial.println();
                          Serial.print(F("WRITING DEBUG-BUFFER (ADD: ") P Add P F(", VAL: ") );
                          for (uint8_t Y = 0; Y < (X + 1); Y++) {
                            PrintBoolHexDec(Val[Y]);
                            if (Y < X) Serial.print('\t');
                          }
                          Serial.println(F(")"));
                          bool ErrorSend = false;
                          uint8_t BufferWriteResult[10];

                          for (uint8_t Y = 0; Y < (X + 1) && !ErrorSend; Y++) {

                            if (RTX_PROTOCAL.Connected()) {
                              uint8_t RTX_TX[RTX_REQ_DEBUG_WRITE_LEN] = RTX_REQ_DEBUG_WRITE(Add + Y, Val[Y]);
                              //              noInterrupts();
                              Serial.flush();
                             uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEBUG_WRITE_LEN, RTX_TX);
                              //              interrupts();
                              if ((WriteStat & B00000001)) {
                                uint32_t RTX_ReadTimeOut = micros();
                                uint8_t BufferWrite[2];
                                bool BufferWriteError;
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
                                      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                                        if ((RX_MSG[0] & B01000000)) {
                                          BufferWriteError = (RX_MSG[0] & B10000000);
                                          if (BufferWriteError) {
                                            BufferWrite[0] = RX_MSG[1];
                                          }
                                          else {
                                            BufferWrite[0] = RX_MSG[1];
                                            BufferWrite[1] = RX_MSG[2];
                                          }
                                          ResponseFound = true;
                                        }
                                      }
                                    }
                                  }
                                }
                                if (ResponseFound) {
                                  if (BufferWriteError) {
                                    Serial.print(F("ERROR: (ADD: ") P BufferWrite[0] P_ F(") OUT OF RANGE"));
                                    Serial.println();
                                    Serial.println();
                                    delay(500);
                                    ErrorSend = true;
                                  }
                                  else {
                                    if (BufferWrite[0] == (Add + Y))
                                      BufferWriteResult[Y] = BufferWrite[1];
                                    else {
                                      Serial.print(F("ERROR: (ADD: ") P BufferWrite[0] P F(" != ") P (Add + Y) P_ F(")"));
                                      Serial.println();
                                      Serial.println();
                                      delay(500);
                                      ErrorSend = true;
                                    }
                                  }
                                }
                                else {
                                  Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                                  Serial.println();
                                  Serial.println();
                                  delay(500);
                                  ErrorSend = true;
                                }
                              }
                              else {
                                Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                                Serial.println();
                                Serial.println();
                                delay(500);
                                ErrorSend = true;
                              }
                            }
                            else {
                              Serial.print(F("ERROR: RTX NOT CONNECTED "));
                              Serial.println();
                              Serial.println();
                              delay(500);
                              ErrorSend = true;
                            }
                          }

                          if (!ErrorSend) {
                            Serial.print(F("DEVICE DEBUG-BUFFER WRITE RESPONSE || WRITE: (ADD: ") P Add P F(", VAL: ") );
                            for (uint8_t Y = 0; Y < (X + 1); Y++) {
                               PrintBoolHexDec(BufferWriteResult[Y]);
                              if (Y < X) Serial.print('\t');
                            }
                            Serial.println(F(")"));
                            Serial.println();
                            Serial.println();
                            delay(500);
                          }

                          CharError = true;
                        }
                      }
                      else {
                        CharError = true;
                      }
                    }
                    else {
                      CharError = true;
                    }
                  }
                }
              }
            }
            else {
              CharAvail = ParseChar(C, 3);
              if (CharAvail = 3 && CharComp("DMP", C, CharAvail) && ParseNext() == 0) {
                Serial.println(F("DEBUG-BUFFER DATA DUMP"));
                Serial.print(F("DOWNLOADING DEBUG-BUFFER DATA FROM IP ") P_ SlaveIP_Selected);
                if (RTX_PROTOCAL.Connected()) {
                  uint8_t RTX_TX[RTX_REQ_DEBUG_BYTES_LEN] = RTX_REQ_DEBUG_BYTES;
                  //              noInterrupts();
                  Serial.flush();
                 uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEBUG_BYTES_LEN, RTX_TX);
                  //              interrupts();
                  if ((WriteStat & B00000001)) {
                    uint32_t RTX_ReadTimeOut = micros();
                    uint16_t TotalBytes;
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
                          if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 6 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 1)) {
                            if ((RX_MSG[0] & B01000000)) {
                              if (!(RX_MSG[0] & B10000000)) {
                                TotalBytes = (RX_MSG[1] + 1);
                              }
                              else {
                                TotalBytes = 0;
                              }
                              ResponseFound = true;
                            }
                          }
                        }
                      }
                    }
                    if (ResponseFound) {
                      //Ping MSG Verified:
                      Serial.print(F("BYTES(DEBUG-BUFFER): ") P_ TotalBytes);
                      Serial.print(F("0%"));
                      for (uint8_t X = 0; X < 92; X++) Serial.print(F(" "));
                      Serial.println(F("100%"));
                      Serial.flush();
                      uint8_t LoadBarCount = 0;
                      uint8_t RTX_DataHold[TotalBytes];
                      for (uint16_t X = 0; X < TotalBytes; X++) {
                        if (Serial.available()) {
                          if (ParseNext() == 2) {
                            char C[1] = {0};
                            uint8_t CharAvail = ParseChar(C, 1);
                            if (ParseNext() == 0) {
                              if (CharAvail == 1 && CharComp("C", C, CharAvail)) {
                                Serial.println();
                                Serial.println(F("DATA DUMP CANCEL"));
                                Serial.println();
                                Serial.println();
                                return BUFF;
                              }
                            }
                          }
                          while (Serial.available()) Serial.read();
                        }
                        if (RTX_PROTOCAL.Connected()) {
                          uint8_t RTX_TX[RTX_REQ_DEBUG_READ_LEN] = RTX_REQ_DEBUG_READ(X);
                          //              noInterrupts();
                          Serial.flush();
                         uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_DEBUG_READ_LEN, RTX_TX);
                          //              interrupts();
                          if ((WriteStat & B00000001)) {
                            uint32_t RTX_ReadTimeOut = micros();
                            uint8_t BufferRead[2];
                            bool BufferReadError;
                            ResponseFound = false;
                            while (micros() - RTX_ReadTimeOut < RTX_RX_TIMEOUT && !ResponseFound) {
                              //TODO Read from RTX, wait for PING response
                              uint8_t RX_IP;
                              uint8_t RX_Bytes;
                              uint8_t RX_MSG[10];
                              if (RTX_PROTOCAL.Read(RX_IP, RX_Bytes, RX_MSG)) {
                                if (RX_IP == SlaveIP_Selected) {
                                  //Corrrect IP
                                  //Make sure it is System MSG and Ping Response:
                                  if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 0 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && (RX_Bytes == 2 || RX_Bytes == 3)) {
                                    if ((RX_MSG[0] & B01000000)) {
                                      BufferReadError = (RX_MSG[0] & B10000000);
                                      if (BufferReadError) {
                                        BufferRead[0] = RX_MSG[1];
                                      }
                                      else {
                                        BufferRead[0] = RX_MSG[1];
                                        BufferRead[1] = RX_MSG[2];
                                      }
                                      ResponseFound = true;
                                    }
                                  }
                                }
                              }
                            }
                            if (ResponseFound) {
                              if (BufferReadError) {
                                Serial.println();
                                Serial.print(F("ERROR: (ADD: ") P BufferRead[0] P_ F(") OUT OF RANGE"));
                                Serial.println();
                                Serial.println();
                                delay(500);
                                return BUFF;
                              }
                              else {
                                if (X == BufferRead[0]) {
                                  RTX_DataHold[X] = BufferRead[1];
                                }
                                else {
                                  Serial.println();
                                  Serial.print(F("ERROR: SYNCING, ") P X P F(" != ") P BufferRead[0]);
                                  Serial.println();
                                  Serial.println();
                                  delay(500);
                                  return BUFF;
                                }
                              }
                            }
                            else {
                              Serial.println();
                              Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                              Serial.println();
                              Serial.println();
                              delay(500);
                              return BUFF;
                            }
                          }
                          else {
                            Serial.println();
                            Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                            Serial.println();
                            Serial.println();
                            delay(500);
                            return BUFF;
                          }
                        }
                        else {
                          Serial.println();
                          Serial.print(F("ERROR: RTX NOT CONNECTED "));
                          Serial.println();
                          Serial.println();
                          delay(500);
                          return BUFF;
                        }
                        while (LoadBarCount < map(X, 0, (TotalBytes - 1), 0, 98)) {
                          Serial.print(F("-"));
                          LoadBarCount++;
                        }
                      }
                      while (LoadBarCount < 98) {
                        Serial.print(F("-"));
                        LoadBarCount++;
                      }
                      Serial.println();
                      Serial.println();
                      Serial.print(F("DEBUG-BUFFER(BYTES: ") P TotalBytes P_ F(")"));
                      for (uint16_t X = 0; X < TotalBytes; X++) {
                        if (!(X % 8)) {
                          Serial.println();
                          Serial.print('\t' P '\t');
                          for (uint8_t Y = 0; Y < 8 && (Y + X) < TotalBytes; Y++) {
                            Serial.print('(' P (Y + X) P ')' P '\t' P '\t' P '\t');
                          }
                          Serial.println();
                          Serial.print(X P F("-") P (X + 7) P ':' P '\t');
                          if (X < 104) Serial.print('\t');
                        }
                        for (uint8_t Y = 0; Y < 8; Y++) {
                          Serial.print(bitRead(RTX_DataHold[X], (7 - Y)));
                        }
                        Serial.print(F("(x"));
                        if (((RTX_DataHold[X] & B11110000) >> 4) > 9) {
                          Serial.write(55 + ((RTX_DataHold[X] & B11110000) >> 4));
                        }
                        else Serial.write(((RTX_DataHold[X] & B11110000) >> 4) + '0');
                        if ((RTX_DataHold[X] & B00001111) > 9) {
                          Serial.write(55 + (RTX_DataHold[X] & B00001111));
                        }
                        else Serial.write((RTX_DataHold[X] & B00001111) + '0');
                        Serial.print(F(")(") P RTX_DataHold[X] P ')' P '\t');
                      }
                      Serial.println();
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                    else {
                      Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                      Serial.println();
                      Serial.println();
                      delay(500);
                    }
                  }
                  else {
                    Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: RTX NOT CONNECTED "));
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
                return BUFF;
              }
            }
          }
          else if (CharAvail == 2 && CharComp("ER", C, CharAvail)) {
            bool EEPROM_ErrorVal = ParseVal();
            if (EEPROM_ErrorVal == 1 && ParseNext() == 0) {
              Serial.println(F("EEPROM ERROR SET"));
              Serial.println();
              Serial.print(F("REQUESTING EEPROM ERROR SET FROM IP: ") P_ SlaveIP_Selected);
              if (RTX_PROTOCAL.Connected()) {
                uint8_t RTX_TX[RTX_REQ_EEPROM_SET_LEN] = RTX_REQ_EEPROM_SET;
                //              noInterrupts();
                Serial.flush();
               uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_SET_LEN, RTX_TX);
                //              interrupts();
                if ((WriteStat & B00000001)) {
                  uint32_t RTX_ReadTimeOut = micros();
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
                        if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 5 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 1) {
                          if ((RX_MSG[0] & B10000000)) {
                            ResponseFound = true;
                          }
                        }
                      }
                    }
                  }
                  if (ResponseFound) {
                    //Ping MSG Verified:
                    Serial.println(F("DEVICE EEPROM ERROR SET RESPONSE || EEPROM ERROR: TRUE"));
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                  else {
                    Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: RTX NOT CONNECTED "));
                Serial.println();
                Serial.println();
                delay(500);
              }
              return BUFF;
            }
            else if (EEPROM_ErrorVal == 0 && ParseNext() == 0) {
              Serial.println(F("EEPROM ERROR RESET"));
              Serial.println();
              Serial.print(F("REQUESTING EEPROM ERROR RESET FROM IP: ") P_ SlaveIP_Selected);
              if (RTX_PROTOCAL.Connected()) {
                uint8_t RTX_TX[RTX_REQ_EEPROM_RESET_LEN] = RTX_REQ_EEPROM_RESET;
                //              noInterrupts();
                Serial.flush();
               uint8_t WriteStat = RTX_PROTOCAL.Write(SlaveIP_Selected, RTX_REQ_EEPROM_RESET_LEN, RTX_TX);
                //              interrupts();
                if ((WriteStat & B00000001)) {
                  uint32_t RTX_ReadTimeOut = micros();
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
                        if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 1 && RTX_PROTOCAL.RTX_MSG_TYPE[1] == 5 && RTX_PROTOCAL.RTX_MSG_TYPE[2] == 1 && RX_Bytes == 1) {
                          if (!(RX_MSG[0] & B10000000)) {
                            ResponseFound = true;
                          }
                        }
                      }
                    }
                  }
                  if (ResponseFound) {
                    //Ping MSG Verified:
                    Serial.println(F("DEVICE EEPROM ERROR RESET RESPONSE || EEPROM ERROR: FALSE"));
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                  else {
                    Serial.print(F("ERROR: TIMEOUT, DEVICE TOOK TO LONG TO RESPOND "));
                    Serial.println();
                    Serial.println();
                    delay(500);
                  }
                }
                else {
                  Serial.print(F("ERROR: CAN'T SEND ") P WriteStat);
                  Serial.println();
                  Serial.println();
                  delay(500);
                }
              }
              else {
                Serial.print(F("ERROR: RTX NOT CONNECTED "));
                Serial.println();
                Serial.println();
                delay(500);
              }
              return BUFF;
            }
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
      Serial.println();
      return BUFF;
    }
    GUI_LOOP_INSERT
  }
}
uint8_t GUI_MSG() {
  Serial.println("GENERAL MSG COMMAND COMING SOON");
  while (1) {
    if (Serial.available()) {
      if (ParseNext() == 2) {
        char C[1] = {0};
        uint8_t CharAvail = ParseChar(C, 1);
        if (ParseNext() == 0) {
          if (CharAvail == 1 && CharComp("C", C, CharAvail)) {
            Serial.println(F("CANCEL"));
            Serial.println();
            Serial.println();
            return CMD_Promt;
          }
          else if (CharAvail == 1 && CharComp("E", C, CharAvail)) {
            Serial.println(F("EXIT"));
            Serial.println();
            Serial.println();
            return IpPromt;
          }
        }
      }
      while (Serial.available()) Serial.read(); //Serial Flush
      Serial.println();
      return MSG;
    }
    GUI_LOOP_INSERT
  }
}


void PrintBoolHexDec(uint8_t Val) {
  for (uint8_t Y = 0; Y < 8; Y++) {
    Serial.print(bitRead(Val, (7 - Y)));
  }
  Serial.print(F("(x"));
  if (((Val & B11110000) >> 4) > 9) {
    Serial.write(55 + ((Val & B11110000) >> 4));
  }
  else Serial.write(((Val & B11110000) >> 4) + '0');
  if ((Val & B00001111) > 9) {
    Serial.write(55 + (Val & B00001111));
  }
  else Serial.write((Val & B00001111) + '0');
  Serial.print(F(")(") P Val P ')');
}


int16_t ParseVal() {
  {
    uint8_t HoldAvailCount = 0;
    while (HoldAvailCount != Serial.available()) {
      delay(1);
      HoldAvailCount = Serial.available();
    }
  }
  int16_t ReturnVal = -1;
  if (Serial.available()) {
    if (Serial.peek() == 'B') {
      //BIN
      Serial.read();
      uint8_t BitCount = 0;
      bool BinHold[15];
      while (1) {
        if (Serial.peek() == '0') {
          if (BitCount < 15) {
            BinHold[BitCount] = false;
            BitCount++;
          }
          Serial.read();
        }
        else if (Serial.peek() == '1') {
          if (BitCount < 15) {
            BinHold[BitCount] = true;
            BitCount++;
          }
          Serial.read();
        }
        else {
          //We are Done:
          if (BitCount) ReturnVal = 0;
          for (uint8_t X = 0; X < BitCount; X++) {
            if (BinHold[(BitCount - 1) - X])
              ReturnVal |= (1 << X);
          }
          return ReturnVal;
        }
      }
    }
    else if (Serial.peek() == '0') {
      Serial.read();
      if (Serial.peek() == 'x') {
        Serial.read();
        uint8_t BitCount = 0;
        uint8_t ByteHold[4];
        while (1) {
          if (Serial.peek() >= '0' && Serial.peek() <= '9') {
            if (BitCount < 4) {
              ByteHold[BitCount] = Serial.peek() - '0';
              BitCount++;
            }
            Serial.read();
          }
          else if (Serial.peek() >= 'A' && Serial.peek() <= 'F') {
            if (BitCount < 4) {
              ByteHold[BitCount] = ((Serial.peek() - 'A') + 10);
              BitCount++;
            }
            Serial.read();
          }
          else {
            if (BitCount) {
              ReturnVal = 0;
              for (uint8_t X = 0; X < BitCount; X++) {
                ReturnVal |= (ByteHold[(BitCount - 1) - X] << (X * 4));
              }
            }
            return ReturnVal;
          }
        }
      }
      else if (Serial.peek() >= '0' && Serial.peek() <= '9') {
        uint8_t BitCount = 0; //Keep Track of how many Number Bits we are recieving in the Serial Buffer
        uint8_t BitHold[5]; //Keep Track of all incoming numbers, max is signed 16 bit(32 some thousand) 5 digits
        while (1) {
          if (Serial.peek() >= '0' && Serial.peek() <= '9') {
            //Cant go over 5 digits:
            if (BitCount < 5) {
              BitHold[BitCount] = (Serial.peek() - '0');
              BitCount++;
            }
            Serial.read();
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
      }
      else return 0;
    }
    else if (Serial.peek() >= '0' && Serial.peek() <= '9') {
      ReturnVal = 0;
      uint8_t BitCount = 0; //Keep Track of how many Number Bits we are recieving in the Serial Buffer
      uint8_t BitHold[5]; //Keep Track of all incoming numbers, max is signed 16 bit(32 some thousand) 5 digits
      while (1) {
        if (Serial.peek() >= '0' && Serial.peek() <= '9') {
          //Cant go over 5 digits:
          if (BitCount < 5) {
            BitHold[BitCount] = (Serial.peek() - '0');
            BitCount++;
          }
          Serial.read();
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
    }
    //DEC
  }
  return ReturnVal;
}

//Returns The Int Found at the Serial Buffer. Returns -1 if no Int found:
int16_t ParseInt() {
  //Delay A little to make3 sure all incoming data made it:
  //TODO: Maybe an Unnecessary Delay? Could Cause Problems
  delay(2);

  int16_t ReturnVal = -1; //Set default as -1
  uint8_t BitCount = 0; //Keep Track of how many Number Bits we are recieving in the Serial Buffer
  uint8_t BitHold[5]; //Keep Track of all incoming numbers, max is signed 16 bit(32 some thousand) 5 digits

  if (Serial.available()) {
    while (1) {
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
  }
  return -1;
}

//Reads a single Char from buffer. Does not read numbers. Returns -1 if not a char or nothing available
uint8_t ParseChar(char *Data, uint8_t MaxSize) {
  //Delay to Allow all Serial Bytes to Come in:
  {
    uint8_t HoldAvailCount = 0;
    while (HoldAvailCount != Serial.available()) {
      delay(1);
      HoldAvailCount = Serial.available();
    }
  }
  uint8_t X = 0;
  for (X = 0; X < MaxSize; X++) {
    if (Serial.available()) {
      //Any Char Between 31 and 127, As long as it is not a Number:
      if ((Serial.peek() > 57 || Serial.peek() < 48) && Serial.peek() >= 32 && Serial.peek() <= 126) {
        Data[X] = Serial.read();
      }
      else return X;
    }
    else return X;
  }
  return X;
}

//Checks what is next availble:
uint8_t ParseNext() {
  if (Serial.peek() >= '0' && Serial.peek() <= '9') return 1; //int Available
  if (Serial.peek() >= 32 && Serial.peek() <= 126) return 2; //Char Available
  return 0; //Nothing or invalid byte available
}

int16_t ParseCharSingle() {
  {
    uint8_t HoldAvailCount = 0;
    while (HoldAvailCount != Serial.available()) {
      delay(1);
      HoldAvailCount = Serial.available();
    }
  }
  if ((Serial.peek() > 57 || Serial.peek() < 48) && Serial.peek() >= 32 && Serial.peek() <= 126) return Serial.read();
  else return -1;
}

bool CharComp(char *C1, char * C2, uint8_t Chars) {
  for (uint8_t X = 0; X < Chars; X++)
    if (C1[X] != C2[X]) return false;
  return true;
}

#define RTX_DEVICE_NAME "ETHER"
#define RTX_SOFTWARE_VERSION {1,2,0}
#define RTX_DEVICE_IP 1

#include "RTX_PROTOCAL.h"
#include "ATEM_UIP.h"
#include "COMPANION_UIP.h"

ATEM_UIP ATEM;
COMP_UIP COMP;

uint8_t COMP_CONNECTION; //0 = Disconnected, 1 = Connected, 2 = ConnectionSleep
bool ATEM_CONNECTION;    //False = Disconnected, True = Connected

uint8_t ATEM_PrevTally[8];
uint32_t RTX_SendTimer = 0;

const uint8_t CameraSwitcherIP[] = {3,4};

void setup() {
  byte ARD_MAC[] = {0x98, 0xA6, 0xAA, 0x15, 0xE8, 0xD9};
  IPAddress ARD_IP(192, 168, 86, 177);

  Ethernet.begin(ARD_MAC, ARD_IP);

  IPAddress COMP_IP(192, 168, 86, 246);
  IPAddress ATEM_IP(192, 168, 86, 68);
  COMP.Setup(COMP_IP, 8000, 8888);
  ATEM.Setup(ATEM_IP, 56321);
}

void loop() {
  //Keep Companion and ATEM Connected and Running:
  COMP_CONNECTION = COMP.Run();
  ATEM_CONNECTION =  ATEM.Run();
  //Check if Tally inputs have changed:
  {
    bool TallyChange = false;
    for (uint8_t X = 0; X < 8; X++) {
      if (ATEM_PrevTally[X] != ATEM.TallyInput[X]) {
        ATEM_PrevTally[X] = ATEM.TallyInput[X];
        TallyChange = true;
      }
    }
    if (TallyChange) {
      //Send Tally Message Interupt
      RTX_Send();
    }
  }
  //Check for incomming messages on RTX:
  {
    uint8_t IP;
    uint8_t Bytes;
    uint8_t MSG[10];
    if (RTX_PROTOCAL.Read(IP, Bytes, MSG)) {
      if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 2 || RTX_PROTOCAL.RTX_MSG_TYPE[0] == 3) {
        //TODO: Dev Message Header System
        //Check if Correct MSG Header:
        if ((MSG[0] & B00011100) == 0) {
          bool VarifiedIP = false;
          for (uint8_t X = 0; X < sizeof(CameraSwitcherIP) && !VarifiedIP; X++) {
            if (CameraSwitcherIP[X] == IP) VarifiedIP = true;
          }
          if (VarifiedIP) {
            COMP.ResetSleep();
            if(MSG[0] & B00100000) {
              //Switch Command Request
              switch((MSG[0] & B11000000) >> 6) {
                case 0:
                {
                  //Companion Instance Switch
                  bool ButtonDir;
                  if((MSG[2] & B01100000) >> 5 == 0) ButtonDir = false;
                  else if ((MSG[2] & B01100000) >> 5 == 1) ButtonDir = true;
                  COMP.PressButton((MSG[1] & B01111111) + 1, (MSG[2] & B00011111) + 1, ButtonDir);
                  COMP.PressButton((MSG[1] & B01111111) + 1, (MSG[2] & B00011111) + 1, ButtonDir);
                  COMP.PressButton((MSG[1] & B01111111) + 1, (MSG[2] & B00011111) + 1, true);
                }
                break;
                case 1:
                {
                  //ATEM Instance Switch
                  //TODO
                }
                break;
                case 2:
                {
                  //N/A
                }
                break;
                case 3:
                {
                  //N/A
                }
                break;
              }
            }
          }
        }
      }
    }
  }
  //Check for RTX Send Interval:
  if (millis() - RTX_SendTimer >= 500) {
    RTX_Send();
  }
}

void RTX_Send() {
  RTX_SendTimer = millis();
  uint8_t TX_MSG[3] = {B00000011,0,0};
  if(COMP_CONNECTION == 1) {
    //Connected
    TX_MSG[0] |= (1 << 5);
  }
  if(COMP_CONNECTION == 2) {
    //Connection Sleep
    TX_MSG[0] |= (1 << 6);
  }
  TX_MSG[0] |= uint8_t(ATEM_CONNECTION << 7);

  for(uint8_t X = 0; X < 8; X++) {
    TX_MSG[1] |= (ATEM.TallyInput[X] & B00000001) << X;
    TX_MSG[2] |= ((ATEM.TallyInput[X] & B00000010) >> 1) << X;
  }
  for(uint8_t X = 0; X < sizeof(CameraSwitcherIP); X++)
  RTX_PROTOCAL.Write(CameraSwitcherIP[X],3,TX_MSG);
}

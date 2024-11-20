#define RTX_DEVICE_NAME "CNTR"
#define RTX_SOFTWARE_VERSION {1,1,0}
#define RTX_DEVICE_IP 3 //3 for Center, 4 for side default

#include "RTX_PROTOCAL.h"
#include "BUTTON_INTERFACE.h"
#include "LED_MACROS.h"

//Create Objects for LEDs:
LED_MACROS LED_StatusOrg(10);
LED_MACROS LED_StatusGrn(11);

LED_MACROS LED_TallyRed(5);
LED_MACROS LED_TallyGrn(6);

MACROS_BUILD LedMacro_0; //Used Generally for LED_StatusGrn Macros
MACROS_BUILD LedMacro_1; //Used Generally for LED_StatusOrg Macros
MACROS_BUILD LedMacro_2; //Used Generally for LED_StatusGrn and LED_StatusOrg Macros

//Objects for Buttons:
void ButtonChange();
void LeftChgBTN();
void CenterChgBTN();
void RightChgBTN();
BUTTON_INTERFACE Button(12, ButtonChange);
BUTTON_INTERFACE LeftBTN(2, LeftChgBTN);
BUTTON_INTERFACE CenterBTN(3, CenterChgBTN);
BUTTON_INTERFACE RightBTN(4, RightChgBTN);

//Assign the Ethernet interface Device IP:
const uint8_t ETHER_IP = 1;

//Used to keep track if Connected
bool TallyConnected;
bool CompConnected;
bool RTX_EtherConnected;

uint32_t RTX_EtherTimer;

//All variables for storing Tally values:
uint8_t TallyStatusPRG;
uint8_t TallyStatusPRE;

const uint8_t TallyStatusMask[] = {B00010000, B00010000};//[0] = PRG, [1] = PRE
bool TallyLEDStatus[2]; //[0] = PRG, [1] = PRE

//Mode of Button Switching:
uint8_t Mode = 0;

void setup() {
  Button.ButtonDebounce(10000);
  //SetUp LED Pins:
  LED_StatusOrg.SetFPS(60);
  LED_StatusGrn.SetFPS(60);

  LED_TallyRed.SetFPS(60);
  LED_TallyGrn.SetFPS(60);
}

void loop() {
  //Variables for Reading RTX MSG's:
  uint8_t RTX_IP;
  uint8_t RTX_Bytes;
  uint8_t RTX_MSG[10];
  if (RTX_PROTOCAL.Read(RTX_IP, RTX_Bytes, RTX_MSG)) {
    //MSG Available
    //Check if MSG Header is either 2 or 3:
    if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 2 || RTX_PROTOCAL.RTX_MSG_TYPE[0] == 3) {
      //Check if Sender IP is Ethernet Interface:
      if (RTX_IP == ETHER_IP) {
        //Check if Packet size is as expected:
        if (RTX_Bytes == 3) {
          //Check if control bytes are 0's:
          if ((RTX_MSG[0] & B00011100) == 0) {
            //Reset RTX Ether Timer:
            RTX_EtherTimer = millis();
            RTX_EtherConnected = true;
            //Get Connection Status's:
            TallyConnected = bool(RTX_MSG[0] & B10000000);
            CompConnected = bool(RTX_MSG[0] & B00100000);

            //Get Tally Status's:
            TallyStatusPRG = RTX_MSG[1];
            TallyStatusPRE = RTX_MSG[2];
            TallyLEDStatus[0] = bool(TallyStatusPRG & TallyStatusMask[0]);
            TallyLEDStatus[1] = bool(TallyStatusPRE & TallyStatusMask[1]);
          }
        }
      }
    }
  }
  //Check if RTX MSG Timeout to long:
  if (millis() - RTX_EtherTimer >= 1100) {
    TallyConnected = false;
    CompConnected = false;
    RTX_EtherConnected = false;
    RTX_EtherTimer = millis();
  }
  LED_RUN();
  Button.Run();
  LeftBTN.Run();
  CenterBTN.Run();
  RightBTN.Run();
}

void LED_RUN() {
  switch (LedMacro_0.Macro) {
    case 0:
      LedMacro_0.PrevMacro = 0;
      break;
    case 1:
      if (LedMacro_0.PrevMacro != LedMacro_0.Macro) {

        LedMacro_0.PrevMacro = LedMacro_0.Macro;
      }
      break;
    case 2:
      if (LedMacro_0.PrevMacro != LedMacro_0.Macro) {
        LedMacro_0.PrevMacro = LedMacro_0.Macro;
      }
      break;
  }


  if (CompConnected) {
    switch (Mode & B00000011) {
      case 0:
        if (LED_StatusGrn.Mode != 1) {
          LED_StatusGrn.Fade(100, 6);
          LED_StatusGrn.Mode = 1;
        }
        if (LED_StatusOrg.Mode != 0) {
          LED_StatusOrg.Fade(0, 6);
          LED_StatusOrg.Mode = 0;
        }
        LED_StatusGrn.Run();
        LED_StatusOrg.Run();
        break;
      case 1:
        if (LED_StatusGrn.Mode < 2 || LED_StatusGrn.Mode > 5) {
          LED_StatusGrn.RST();
          LED_StatusGrn.Mode = 2;
        }
        if (LED_StatusOrg.Mode != 0) {
          LED_StatusOrg.Fade(0, 6);
          LED_StatusOrg.Mode = 0;
        }
        if (LED_StatusGrn.Run()) {
          switch (LED_StatusGrn.Mode) {
            case 2:
              LED_StatusGrn.Fade(100, 12);
              LED_StatusGrn.Mode = 3;
              break;
            case 3:
              LED_StatusGrn.Set(50);
              LED_StatusGrn.Mode = 4;
              break;
            case 4:
              LED_StatusGrn.Fade(10, 12);
              LED_StatusGrn.Mode = 5;
              break;
            case 5:
              LED_StatusGrn.Set(50);
              LED_StatusGrn.Mode = 2;
              break;
          }
        }
        LED_StatusOrg.Run();
        break;
      case 2:
        if (LED_StatusGrn.Mode != 0) {
          LED_StatusGrn.Fade(0, 6);
          LED_StatusGrn.Mode = 0;
        }
        if (LED_StatusOrg.Mode != 1) {
          LED_StatusOrg.Fade(100, 6);
          LED_StatusOrg.Mode = 1;
        }
        LED_StatusGrn.Run();
        LED_StatusOrg.Run();
        break;
      case 3:
        if (LED_StatusGrn.Mode != 0) {
          LED_StatusGrn.Fade(0, 6);
          LED_StatusGrn.Mode = 0;
        }
        if (LED_StatusOrg.Mode < 2 || LED_StatusOrg.Mode > 5) {
          LED_StatusOrg.RST();
          LED_StatusOrg.Mode = 2;
        }
        LED_StatusGrn.Run();
        if (LED_StatusOrg.Run()) {
          switch (LED_StatusOrg.Mode) {
            case 2:
              LED_StatusOrg.Fade(100, 12);
              LED_StatusOrg.Mode = 3;
              break;
            case 3:
              LED_StatusOrg.Set(50);
              LED_StatusOrg.Mode = 4;
              break;
            case 4:
              LED_StatusOrg.Fade(10, 12);
              LED_StatusOrg.Mode = 5;
              break;
            case 5:
              LED_StatusOrg.Set(50);
              LED_StatusOrg.Mode = 2;
              break;
          }
        }
        break;
    }
  }
  else {
    if (LED_StatusGrn.Mode != 0) {
      LED_StatusGrn.Fade(0, 6);
      LED_StatusGrn.Mode = 0;
    }
    if (LED_StatusOrg.Mode < 6 || LED_StatusOrg.Mode > 9) {
      LED_StatusOrg.Set(0, 200);
      LED_StatusOrg.Mode = 6;
    }
    LED_StatusGrn.Run();
    if (LED_StatusOrg.Run()) {
      switch (LED_StatusOrg.Mode) {
        case 6:
          LED_StatusOrg.Set(100, 50);
          LED_StatusOrg.Mode = 7;
          break;
        case 7:
          LED_StatusOrg.Set(0, 100);
          LED_StatusOrg.Mode = 8;
          break;
        case 8:
          LED_StatusOrg.Set(100, 50);
          LED_StatusOrg.Mode = 9;
          break;
        case 9:
          LED_StatusOrg.Set(0, 500);
          LED_StatusOrg.Mode = 6;
          break;
      }
    }
  }
  if (TallyConnected) {
    if (TallyLEDStatus[0]) {
      if (LED_TallyRed.Mode != 1) {
        LED_TallyRed.Fade(100, 3);
        LED_TallyRed.Mode = 1;
      }
    }
    else {
      if (LED_TallyRed.Mode != 0) {
        LED_TallyRed.Fade(0, 3);
        LED_TallyRed.Mode = 0;
      }
    }
    LED_TallyRed.Run();
  }
  else {
    if (LED_TallyRed.Mode < 2 || LED_TallyRed.Mode > 5) {
      LED_TallyRed.Set(0, 200);
      LED_TallyRed.Mode = 2;
    }
    if (LED_TallyRed.Run()) {
      switch (LED_TallyRed.Mode) {
        case 2:
          LED_TallyRed.Set(100, 50);
          LED_TallyRed.Mode = 3;
          break;
        case 3:
          LED_TallyRed.Set(0, 100);
          LED_TallyRed.Mode = 4;
          break;
        case 4:
          LED_TallyRed.Set(100, 50);
          LED_TallyRed.Mode = 5;
          break;
        case 5:
          LED_TallyRed.Set(0, 500);
          LED_TallyRed.Mode = 2;
          break;
      }
    }
  }
}

void ButtonChange() {
  if (!Button.ButtonState()) {
    Mode++;
  }

}

void LeftChgBTN() {
  if (!LeftBTN.ButtonState()) {
    byte TX_MSG[3] = {B00100011, B00000100, 26};
    RTX_PROTOCAL.Write(ETHER_IP, 3, TX_MSG);
  }
}
void CenterChgBTN() {
  if (!CenterBTN.ButtonState()) {
    byte TX_MSG[3] = {B00100011, B00000100, 26};
    RTX_PROTOCAL.Write(ETHER_IP, 3, TX_MSG);
  }
}
void RightChgBTN() {
  if (!RightBTN.ButtonState()) {
    byte TX_MSG[3] = {B00100011, B00000100, 26};
    RTX_PROTOCAL.Write(ETHER_IP, 3, TX_MSG);
  }
}

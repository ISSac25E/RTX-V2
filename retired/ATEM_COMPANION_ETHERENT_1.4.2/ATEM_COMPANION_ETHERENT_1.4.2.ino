#define RTX_DEVICE_NAME "ETHER"
#define RTX_SOFTWARE_VERSION {1, 4, 1}
#define RTX_DEVICE_IP 1

#define RTX_EEPROM_SECTORS 6
#define RTX_DEBUG_SECTORS 2

#include "RTX_PROTOCAL.h"
#include "ATEM_UIP_FAST.h"
#include "COMPANION_UIP.h"

//Makes it a little easier
#define CONFIG_BUFF(ADD) (RTX_PROTOCAL.RTX_EEPROM_BUFFER[ADD])
#define DEBUG_BUFF(ADD) (RTX_PROTOCAL.RTX_DEBUG_BUFFER[ADD])

//Setup Companion and ATEM Objects
ATEM_UIP_FAST ATEM;
COMP_UIP COMP;

uint8_t COMP_CONNECTION; //0 = Disconnected, 1 = Connected, 2 = ConnectionSleep
bool ATEM_CONNECTION;    //False = Disconnected, True = Connected

uint8_t ATEM_PrevTally[8];  //This will be used to Check if tallies change
uint32_t RTX_SendTimer = 0; //This is to Send RTX Messages at an Interval

void setup() {
  //If No EEPROM Error, then Setup Ethernet
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR) EthernetSetup();
  //else wait until Error cleared
}

void loop() {
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR) {

    {
      uint8_t CompConnected = 0;
      bool ATEMConnected = false;

        if (CONFIG_BUFF(7) & B00000010)
          CompConnected = COMP.Run();
        if (CONFIG_BUFF(7) & B00000100)
          ATEMConnected = ATEM.Run();
      if (CompConnected != COMP_CONNECTION || ATEMConnected != ATEM_CONNECTION) {
        COMP_CONNECTION = CompConnected;
        ATEM_CONNECTION = ATEMConnected;
        //Connection States have change, Update the Devices Right Away
        //Only send if RTX Master Send is Enabled:
        if ((CONFIG_BUFF(34) & B00000010)) {
          RTX_Send();
        }
      }
      else {
        COMP_CONNECTION = CompConnected;
        ATEM_CONNECTION = ATEMConnected;
      }
    }

    //Check if Tally inputs have changed:
    {
      bool TallyChange = false;
      for (uint8_t X = 0; X < 8 && !TallyChange; X++) {
        if (ATEM_PrevTally[X] != ATEM.TallyInput[X]) {
          ATEM_PrevTally[X] = ATEM.TallyInput[X];
          TallyChange = true;
        }
      }
      if (TallyChange) {
        //Send Tally Message Interupt
        //Only send if RTX Master Send is Enabled:
        if ((CONFIG_BUFF(34) & B00000010)) {
          RTX_Send();
        }
      }
    }
    //Update Debug Sleep Timer:
    {
      uint16_t COMP_SleepTimer = COMP.SleepTimer();
      DEBUG_BUFF(1) = (COMP_SleepTimer % 60); //Sec
      DEBUG_BUFF(0) = (COMP_SleepTimer / 60); //Min
    }
    //Update Debug Ethernet Connection:
    {
      DEBUG_BUFF(2) = (ATEM_CONNECTION << 2);
      if (COMP_CONNECTION == 1) {
        //Connected
        DEBUG_BUFF(2) |= 1;
      }
      if (COMP_CONNECTION == 2) {
        //Connection Sleep
        DEBUG_BUFF(2) |= (1 << 1);
      }
    }
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(3) = 0;  //PRG
      DEBUG_BUFF(4) = 0;  //PRE
      for (uint8_t X = 0; X < 8; X++) {
        DEBUG_BUFF(3) |= (ATEM.TallyInput[X] & B00000001) << X;
        DEBUG_BUFF(4) |= ((ATEM.TallyInput[X] & B00000010) >> 1) << X;
      }
    }
    //Update Total Run Time in Debug Registers:
    {
      uint32_t MillisHold = millis();
      DEBUG_BUFF(8) = (MillisHold / 3600000L);
      DEBUG_BUFF(9) = ((MillisHold % 3600000L) / 60000L);
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
          //Check if Master SWTCH is Enabled:
          if ((CONFIG_BUFF(34) & B00000100)) {
            bool VarifiedIP = false;
            for (uint8_t X = 0; X < RTX_Devices() && !VarifiedIP; X++) {
              //Check if correct IP and Is Enabled
              if (RTX_Devices(X) == IP && RTX_Devices_SWC_EN(X))
                VarifiedIP = true;
            }
            if (VarifiedIP) {
              if ((MSG[0] & B00011100) == 0) {
                COMP.ResetSleep();
                if (MSG[0] & B00100000) {
                  //Switch Command Request
                  DEBUG_BUFF(14)++;
                  switch ((MSG[0] & B11000000) >> 6) {
                    case 0:
                      {
                        //Companion Instance Switch
                        bool ButtonDir = false;
                        if ((MSG[2] & B01100000) >> 5 == 0) ButtonDir = false;
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
                        //MACRO Instance?
                        //TODO
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
    }
    //Check for new commands from Debug Buffer:
    {
      // DEBUG_BUFF(7);
      if (DEBUG_BUFF(7) & B00000001) {
        EthernetSetup();
      }
      if (DEBUG_BUFF(7) & B00000010) {
        COMP.ResetSleep();
      }
      DEBUG_BUFF(7) &= B11111100;
    }
    //Check for RTX Send Interval:
    {
      //Only send if RTX Master Send is Enabled:
      if ((CONFIG_BUFF(34) & B00000010)) {
        uint16_t TimeDelay = (CONFIG_BUFF(33) | (CONFIG_BUFF(32) << 8));
        //Only Send RTX Messages is Timer is not a Zero
        if (TimeDelay) {
          if (millis() - RTX_SendTimer >= TimeDelay) {
            RTX_Send();
          }
        }
      }
    }
  }
  else {
    uint8_t CompConnected = 0;
    bool ATEMConnected = false;
    
    //Update Debug Sleep Timer:
    {
      DEBUG_BUFF(1) = 0;
      DEBUG_BUFF(0) = 0;
    }
    //Update Debug Ethernet Connection:
    {
        DEBUG_BUFF(2) = 0;
    }
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(3) = 0;  //PRG
      DEBUG_BUFF(4) = 0;  //PRE
      for (uint8_t X = 0; X < 8; X++) {
        DEBUG_BUFF(3) |= (ATEM.TallyInput[X] & B00000001) << X;
        DEBUG_BUFF(4) |= ((ATEM.TallyInput[X] & B00000010) >> 1) << X;
      }
    }
    //Update Total Run Time in Debug Registers:
    {
      uint32_t MillisHold = millis();
      DEBUG_BUFF(8) = (MillisHold / 3600000L);
      DEBUG_BUFF(9) = ((MillisHold % 3600000L) / 60000L);
    }
    RTX_PROTOCAL.Run();
  }
}

void EthernetSetup() {
  byte ARD_MAC[] = {
    CONFIG_BUFF(0), CONFIG_BUFF(1), CONFIG_BUFF(2),
    CONFIG_BUFF(3), CONFIG_BUFF(4), CONFIG_BUFF(5)
  };
  IPAddress ARD_IP(CONFIG_BUFF(8), CONFIG_BUFF(9), CONFIG_BUFF(10), CONFIG_BUFF(11));

  Ethernet.begin(ARD_MAC, ARD_IP);

  //Setup Companion Connection:
  if (CONFIG_BUFF(7) & B00000010) {
    IPAddress COMP_IP(CONFIG_BUFF(24), CONFIG_BUFF(25), CONFIG_BUFF(26), CONFIG_BUFF(27));
    int16_t COMP_RX_Port = (CONFIG_BUFF(29) | (CONFIG_BUFF(28) << 8));
    uint16_t COMP_TX_Port = (CONFIG_BUFF(31) | (CONFIG_BUFF(30) << 8));
//    uint16_t ConnectionSleep = (CONFIG_BUFF(37) | (CONFIG_BUFF(36) << 8));
    COMP.SetConnectButton(((CONFIG_BUFF(38) & B01111111) + 1), ((CONFIG_BUFF(39) & B00011111) + 1));
    COMP.ConnectionSleep((CONFIG_BUFF(37) | (CONFIG_BUFF(36) << 8)));
    COMP.ResetSleep();
    COMP.Setup(COMP_IP, COMP_RX_Port, COMP_TX_Port);
  }
  else COMP.Disconnect();

  //Setup ATEM Connection:
  if (CONFIG_BUFF(7) & B00000100) {
    IPAddress ATEM_IP(CONFIG_BUFF(16), CONFIG_BUFF(17), CONFIG_BUFF(18), CONFIG_BUFF(19));
    uint16_t ATEM_Port = (CONFIG_BUFF(23) | (CONFIG_BUFF(22) << 8));
    ATEM.Setup(ATEM_IP, ATEM_Port);
  }
  else ATEM.Disconnect();
}

void RTX_Send() {
  RTX_SendTimer = millis();
  uint8_t TX_MSG[3] = {B00000011, 0, 0};
  if (COMP_CONNECTION == 1) {
    //Connected
    TX_MSG[0] |= (1 << 5);
  }
  if (COMP_CONNECTION == 2) {
    //Connection Sleep
    TX_MSG[0] |= (1 << 6);
  }
  TX_MSG[0] |= uint8_t(ATEM_CONNECTION << 7);

  for (uint8_t X = 0; X < 8; X++) {
    TX_MSG[1] |= (ATEM.TallyInput[X] & B00000001) << X;
    TX_MSG[2] |= ((ATEM.TallyInput[X] & B00000010) >> 1) << X;
  }
  for (uint8_t X = 0; X < RTX_Devices(); X++) {
    //Check if Send is Enabled for the specific device:
    if (RTX_Devices_SND_EN(X))
      RTX_PROTOCAL.Write(RTX_Devices(X), 3, TX_MSG);
  }
}

//Returns Number of Configured RTX Devices
uint8_t RTX_Devices() {
  uint8_t RTX_DeviceCount = 0;
  for (uint8_t X = 0; X < 8; X++) {
    if (CONFIG_BUFF(40 + X) & B10000000) RTX_DeviceCount++;
  }
  return RTX_DeviceCount;
}

//Returns Actual IP of Device as Configured
uint8_t RTX_Devices(uint8_t Device) {
  for (uint8_t X = (Device & B00000111); X < 8; X++) {
    if (CONFIG_BUFF(40 + X) & B10000000) {
      return (CONFIG_BUFF(40 + X) & B00000111);
    }
  }
  return 0;
}

//Check if Send is Enabled:
bool RTX_Devices_SND_EN(uint8_t Device) {
  for (uint8_t X = (Device & B00000111); X < 8; X++) {
    if (CONFIG_BUFF(40 + X) & B10000000) {
      return (CONFIG_BUFF(40 + X) & B00100000);
    }
  }
  return false;
}

//Check if Switch is enabled:
bool RTX_Devices_SWC_EN(uint8_t Device) {
  for (uint8_t X = (Device & B00000111); X < 8; X++) {
    if (CONFIG_BUFF(40 + X) & B10000000) {
      return (CONFIG_BUFF(40 + X) & B01000000);
    }
  }
  return false;
}

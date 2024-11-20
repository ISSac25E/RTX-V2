#define RTX_DEVICE_NAME "CNTR"
#define RTX_SOFTWARE_VERSION {1,2,0}
#define RTX_DEVICE_IP 3 //3 for Center, 4 for side default

#define RTX_EEPROM_SECTORS 25  //256 Bytes of Config
#define RTX_DEBUG_SECTORS 2   //16 Bytes of Debug and Control

#include "RTX_PROTOCAL.h"
#include "BUTTON_INTERFACE.h"
#include "NEO_PIXEL.h"
#include "LED_MACROS.h"

//All Objects for LEDs:
MACROS  StatusMacros[3];   //One for each color [0]R, [1]G, [2]B
MACROS_BUILD  StatusBuild;

MACROS  TallyMacros[3];   //One for each color [0]R, [1]G, [2]B
MACROS_BUILD  TallyBuild;

MACROS  ErrorMacro[3];   //One for each color [0]R, [1]G, [2]B
MACROS_BUILD  ErrorBuild;

NEO_PIXEL_DRIVER PixelDriver(12);
//NEO_PIXEL StatusGreen(&PixelDriver);
//NEO_PIXEL StatusRed(&PixelDriver);
//NEO_PIXEL StatusBlue(&PixelDriver);

NEO_PIXEL TallyGreen(&PixelDriver);
NEO_PIXEL TallyRed(&PixelDriver);
NEO_PIXEL TallyBlue(&PixelDriver);

NEO_PIXEL StatusGreen(&PixelDriver);
NEO_PIXEL StatusRed(&PixelDriver);
NEO_PIXEL StatusBlue(&PixelDriver);

//Objects for Buttons and Button Macros:
PIN_DRIVER ModePin(3);
PIN_MACRO ModeMacro;

//Makes it a little easier:
#define CONFIG_BUFF(ADD) (RTX_PROTOCAL.RTX_EEPROM_BUFFER[ADD])
#define DEBUG_BUFF(ADD) (RTX_PROTOCAL.RTX_DEBUG_BUFFER[ADD])

//Variable for Button Macro Mode
uint8_t Mode = 0;

uint8_t COMP_CONNECTION = 0;
bool ATEM_CONNECTION = false;
bool RTX_LINK = false;  //If Connected to RTX BUS and Ether IP Linked
uint32_t RTX_LINK_TIMER = 0;

uint8_t ATEM_TALLY_PRG = 0;
uint8_t ATEM_TALLY_PRE = 0;

void setup() {
  PixelDriver.InitPixel();
  for (uint8_t X = 0; X < 3; X++) StatusMacros[X].SetFPS(60);
  for (uint8_t X = 0; X < 3; X++) TallyMacros[X].SetFPS(60);
}

void loop() {
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR) {
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(1) = ATEM_TALLY_PRG;  //PRG
      DEBUG_BUFF(2) = ATEM_TALLY_PRE;  //PRE
    }
    //Update Total Run Time in Debug Registers:
    {
      uint32_t MillisHold = millis();
      DEBUG_BUFF(8) = (MillisHold / 3600000L);
      DEBUG_BUFF(9) = ((MillisHold % 3600000L) / 60000L);
    }
    //Update Connection Status in Debug Buffer:
    {
      DEBUG_BUFF(7) = COMP_CONNECTION;
      DEBUG_BUFF(7) |= (ATEM_CONNECTION << 2);
      DEBUG_BUFF(7) |= (RTX_LINK << 7);
    }
    //Check for incomming messages on RTX:
    {
      uint8_t IP;
      uint8_t Bytes;
      uint8_t MSG[10];
      if (RTX_PROTOCAL.Read(IP, Bytes, MSG)) {
        if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 2 || RTX_PROTOCAL.RTX_MSG_TYPE[0] == 3) {
          //Verify if the Message is comming from Ethernet:
          if (IP == (CONFIG_BUFF(199) & B00000111)) {
            if (Bytes == 3 && !(MSG[0] & B00011100)) {
              RTX_LINK = true;
              RTX_LINK_TIMER = millis();
              COMP_CONNECTION = ((MSG[0] & B01100000) >> 5);
              ATEM_CONNECTION = (MSG[0] & B10000000);
              ATEM_TALLY_PRG = MSG[1];
              ATEM_TALLY_PRE = MSG[2];
            }
          }
        }
      }
    }
    {
      uint16_t TimerHold = (CONFIG_BUFF(191) | (CONFIG_BUFF(190) << 8));
      if (TimerHold && RTX_LINK) {
        if (millis() - RTX_LINK_TIMER >= TimerHold) {
          RTX_LINK = false;
          COMP_CONNECTION = 0;
          ATEM_CONNECTION = false;
        }
      }
    }
  }
  else {
    RTX_LINK = false;
    COMP_CONNECTION = 0;
    ATEM_CONNECTION = false;
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(1) = ATEM_TALLY_PRG;  //PRG
      DEBUG_BUFF(2) = ATEM_TALLY_PRE;  //PRE
    }
    //Update Total Run Time in Debug Registers:
    {
      uint32_t MillisHold = millis();
      DEBUG_BUFF(8) = (MillisHold / 3600000L);
      DEBUG_BUFF(9) = ((MillisHold % 3600000L) / 60000L);
    }
    //Update Connection Status in Debug Buffer:
    {
      DEBUG_BUFF(7) = 0;
    }
    RTX_PROTOCAL.Run();
  }
  BUTTON_RUN();
  LED_RUN();
}

void BUTTON_RUN() {
  if (ModeMacro.Run(ModePin.Run())) {
    if (!ModeMacro.State()) {
      Mode++;
    }
  }
}

void LED_RUN() {
  //Set which macro should be running here:
  {
    {
      uint8_t ErrorMacroMode = 0;
      if (COMP_CONNECTION != 1) ErrorMacroMode = 1;
      ErrorMacroMode |= (!ATEM_CONNECTION << 1);
      ErrorBuild.Macro = ErrorMacroMode;
    }
    StatusBuild.Macro = ((Mode % 4) + 1);
//    TallyBuild.Macro = (Mode % 4);
    {
      uint8_t TallyMode = 0;
      if (ATEM_TALLY_PRG & B00010000) {
        TallyMode = 1;
      }
      if (ATEM_TALLY_PRE & B00010000) {
        TallyMode |= B10;
      }
      TallyBuild.Macro = TallyMode;
    }
  }

  TallyRed.Set(TallyMacros[0].Val());
  TallyGreen.Set(TallyMacros[1].Val());
  TallyBlue.Set(TallyMacros[2].Val());

  if (StatusRed.MacroType == 0)
    StatusRed.Set(StatusMacros[0].Val());
  else if (StatusRed.MacroType == 1)
    StatusRed.Set(ErrorMacro[0].Val());

  if (StatusGreen.MacroType == 0)
    StatusGreen.Set(StatusMacros[1].Val());
  else if (StatusGreen.MacroType == 1)
    StatusGreen.Set(ErrorMacro[1].Val());

  if (StatusBlue.MacroType == 0)
    StatusBlue.Set(StatusMacros[2].Val());
  else if (StatusBlue.MacroType == 1)
    StatusBlue.Set(ErrorMacro[2].Val());

  static uint32_t PixelTimer = millis();
  if (PixelDriver.WritePixel) {
    PixelTimer = millis();
  }
  else {
    if (millis() - PixelTimer >= 100) {
      PixelDriver.WritePixel = true;
    }
  }
  PixelDriver.Write();
  switch (StatusBuild.Macro) {
    case 0:
      //Led Off not running anything
      if (StatusBuild.MacroChange()) {
        for (uint8_t X = 0; X < 3; X++) StatusMacros[X].Fade(0, 12); //12 Frames, 12/60FPS: 1/5(second)
      }
      StatusMacros[0].Run();
      StatusMacros[1].Run();
      StatusMacros[2].Run();
      break;
    case 1:
      if (StatusBuild.MacroChange()) {
        StatusMacros[0].Fade(0, 12);
        StatusMacros[1].Fade(0, 12);
        StatusMacros[2].Fade(0, 12);
      }
      StatusMacros[0].Run();
      StatusMacros[1].Run();
      StatusMacros[2].Run();
      break;
    case 2:
      if (StatusBuild.MacroChange()) {
        StatusMacros[0].Fade(255, 12);
        StatusMacros[1].Fade(0, 12);
        StatusMacros[2].Fade(0, 12);
      }
      StatusMacros[0].Run();
      StatusMacros[1].Run();
      StatusMacros[2].Run();
      break;
    case 3:
      if (StatusBuild.MacroChange()) {
        StatusMacros[0].Fade(0, 12);
        StatusMacros[1].Fade(255, 12);
        StatusMacros[2].Fade(255, 12);
      }
      StatusMacros[0].Run();
      StatusMacros[1].Run();
      StatusMacros[2].Run();
      break;
    case 4:
      if (StatusBuild.MacroChange()) {
        StatusMacros[0].Fade(160, 12);
        StatusMacros[1].Fade(0, 12);
        StatusMacros[2].Fade(255, 12);
      }
      StatusMacros[0].Run();
      StatusMacros[1].Run();
      StatusMacros[2].Run();
      break;
  }
  switch (ErrorBuild.Macro) {
    case 0:
      if (ErrorBuild.MacroChange()) {
        StatusRed.MacroType = 0;
        StatusGreen.MacroType = 0;
        StatusBlue.MacroType = 0;
      }
      break;
    case 1:
      //COMP Not Connected
      {
        if (ErrorBuild.MacroChange()) {
          ErrorBuild.MacroStage = 0;
          ErrorMacro[0].RST();
          ErrorMacro[1].RST();
          ErrorMacro[2].RST();

          ErrorMacro[0].Set(0, 500);
          ErrorMacro[1].Set(0, 500);
          ErrorMacro[2].SetVal(0);
        }

        ErrorMacro[0].Run();
        ErrorMacro[1].Run();
        ErrorMacro[2].Run();

        if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready()) {
          switch ((ErrorBuild.MacroStage % 6)) {
            case 0:
              ErrorMacro[0].Set(0, 50);
              StatusRed.MacroType = 1;
              StatusGreen.MacroType = 1;
              StatusBlue.MacroType = 1;

              ErrorBuild.MacroStage++;
              break;
            case 1:
              ErrorMacro[0].Set(255, 50);
              ErrorMacro[1].Set(160, 50);

              ErrorBuild.MacroStage++;
              break;
            case 2:
              ErrorMacro[0].Set(0, 100);
              ErrorMacro[1].Set(0, 100);

              ErrorBuild.MacroStage++;
              break;
            case 3:
              ErrorMacro[0].Set(255, 50);
              ErrorMacro[1].Set(160, 50);

              ErrorBuild.MacroStage++;
              break;
            case 4:
              ErrorMacro[0].Set(0, 50);
              ErrorMacro[1].Set(0, 50);

              ErrorBuild.MacroStage++;
              break;
            case 5:
              ErrorMacro[0].Set(0, 500);
              ErrorMacro[1].Set(0, 500);
              StatusRed.MacroType = 0;
              StatusGreen.MacroType = 0;
              StatusBlue.MacroType = 0;

              ErrorBuild.MacroStage = 0;
              break;
            default:
              ErrorBuild.MacroStage = 0;
              break;
          }
        }
      }
      break;
    case 2:
      //ATEM Not Connected
      {
        if (ErrorBuild.MacroChange()) {
          ErrorBuild.MacroStage = 0;
          ErrorMacro[0].RST();
          ErrorMacro[1].RST();
          ErrorMacro[2].RST();

          ErrorMacro[0].Set(0, 500);
          ErrorMacro[1].SetVal(0);
          ErrorMacro[2].SetVal(0);
        }

        ErrorMacro[0].Run();
        ErrorMacro[1].Run();
        ErrorMacro[2].Run();

        if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready()) {
          switch (ErrorBuild.MacroStage) {
            case 0:
              ErrorMacro[0].Set(0, 50);
              StatusRed.MacroType = 1;
              StatusGreen.MacroType = 1;
              StatusBlue.MacroType = 1;

              ErrorBuild.MacroStage++;
              break;
            case 1:
              ErrorMacro[0].Set(255, 50);

              ErrorBuild.MacroStage++;
              break;
            case 2:
              ErrorMacro[0].Set(0, 100);

              ErrorBuild.MacroStage++;
              break;
            case 3:
              ErrorMacro[0].Set(255, 50);

              ErrorBuild.MacroStage++;
              break;
            case 4:
              ErrorMacro[0].Set(0, 50);

              ErrorBuild.MacroStage++;
              break;
            case 5:
              ErrorMacro[0].Set(0, 500);
              StatusRed.MacroType = 0;
              StatusGreen.MacroType = 0;
              StatusBlue.MacroType = 0;

              ErrorBuild.MacroStage = 0;
              break;
            default:
              ErrorBuild.MacroStage = 0;
              break;
          }
        }
      }
      break;
    case 3:
      //ATEM && COMP Not Connected
      {
        if (ErrorBuild.MacroChange()) {
          ErrorBuild.MacroStage = 0;
          ErrorMacro[0].RST();
          ErrorMacro[1].RST();
          ErrorMacro[2].RST();

          ErrorMacro[0].Set(0, 500);
          ErrorMacro[1].Set(0, 500);
          ErrorMacro[2].SetVal(0);
        }

        ErrorMacro[0].Run();
        ErrorMacro[1].Run();
        ErrorMacro[2].Run();

        if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready()) {
          switch (ErrorBuild.MacroStage) {
            case 0:
              ErrorMacro[0].Set(0, 50);
              StatusRed.MacroType = 1;
              StatusGreen.MacroType = 1;
              StatusBlue.MacroType = 1;

              ErrorBuild.MacroStage++;
              break;
            case 1:
              ErrorMacro[0].Set(255, 50);
              ErrorMacro[1].Set(160, 50);

              ErrorBuild.MacroStage++;
              break;
            case 2:
              ErrorMacro[0].Set(0, 100);
              ErrorMacro[1].Set(0, 100);

              ErrorBuild.MacroStage++;
              break;
            case 3:
              ErrorMacro[0].Set(255, 50);

              ErrorBuild.MacroStage++;
              break;
            case 4:
              ErrorMacro[0].Set(0, 100);
              ErrorMacro[1].Set(0, 100);

              ErrorBuild.MacroStage++;
              break;
            case 5:
              ErrorMacro[0].Set(255, 50);
              ErrorMacro[1].Set(160, 50);

              ErrorBuild.MacroStage++;
              break;
            case 6:
              ErrorMacro[0].Set(0, 100);
              ErrorMacro[1].Set(0, 100);

              ErrorBuild.MacroStage++;
              break;
            case 7:
              ErrorMacro[0].Set(255, 50);

              ErrorBuild.MacroStage++;
              break;
            case 8:
              ErrorMacro[0].Set(0, 50);

              ErrorBuild.MacroStage++;
              break;
            case 9:
              ErrorMacro[0].Set(0, 500);
              StatusRed.MacroType = 0;
              StatusGreen.MacroType = 0;
              StatusBlue.MacroType = 0;

              ErrorBuild.MacroStage = 0;
              break;
            default:
              ErrorBuild.MacroStage = 0;
              break;
          }
        }
      }
      break;
  }
  switch (TallyBuild.Macro) {
    case 0:
      //Led Off not running anything
      if (TallyBuild.MacroChange()) {
        for (uint8_t X = 0; X < 3; X++) TallyMacros[X].Fade(0, 6); //12 Frames, 12/60FPS: 1/5(second)
      }
      TallyMacros[0].Run();
      TallyMacros[1].Run();
      TallyMacros[2].Run();
      break;
    case 1:
      if (TallyBuild.MacroChange()) {
        TallyMacros[0].Fade(255, 6);
        TallyMacros[1].Fade(0, 6);
        TallyMacros[2].Fade(0, 6);
      }
      TallyMacros[0].Run();
      TallyMacros[1].Run();
      TallyMacros[2].Run();
      break;
    case 2:
      if (TallyBuild.MacroChange()) {
        TallyMacros[0].Fade(0, 6);
        TallyMacros[1].Fade(255, 6);
        TallyMacros[2].Fade(0, 6);
      }
      TallyMacros[0].Run();
      TallyMacros[1].Run();
      TallyMacros[2].Run();
      break;
    case 3:
      if (TallyBuild.MacroChange()) {
        TallyMacros[0].Fade(255, 6);
        TallyMacros[1].Fade(255, 6);
        TallyMacros[2].Fade(0, 6);
      }
      TallyMacros[0].Run();
      TallyMacros[1].Run();
      TallyMacros[2].Run();
      break;
    case 4:
      if (TallyBuild.MacroChange()) {
        TallyMacros[0].Fade(160, 12);
        TallyMacros[1].Fade(0, 12);
        TallyMacros[2].Fade(255, 12);
      }
      TallyMacros[0].Run();
      TallyMacros[1].Run();
      TallyMacros[2].Run();
      break;
  }
}

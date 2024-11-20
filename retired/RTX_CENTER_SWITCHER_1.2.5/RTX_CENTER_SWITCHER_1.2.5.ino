#define RTX_DEVICE_NAME "CNTR"
#define RTX_SOFTWARE_VERSION \
  {                          \
    1, 2, 5                  \
  }
#define RTX_DEVICE_IP 3 //3 for Center, 4 for side default

#define RTX_EEPROM_SECTORS 25 //256 Bytes of Config
#define RTX_DEBUG_SECTORS 2   //16 Bytes of Debug and Control

#include "RTX_PROTOCAL.h"
#include "BUTTON_INTERFACE.h"
#include "NEO_PIXEL.h"
#include "LED_MACROS.h"

//All Objects for LEDs:
MACROS StatusMacros[3]; //One for each color [0]R, [1]G, [2]B
MACROS_BUILD StatusBuild;

MACROS TallyMacros[3]; //One for each color [0]R, [1]G, [2]B
MACROS_BUILD TallyBuild;

MACROS ErrorMacro[3]; //One for each color [0]R, [1]G, [2]B for both Tally and Status
MACROS_BUILD ErrorBuild;

NEO_PIXEL_DRIVER PixelDriver(12);

NEO_PIXEL StatusGreen(&PixelDriver);
NEO_PIXEL StatusRed(&PixelDriver);
NEO_PIXEL StatusBlue(&PixelDriver);

NEO_PIXEL TallyGreen(&PixelDriver);
NEO_PIXEL TallyRed(&PixelDriver);
NEO_PIXEL TallyBlue(&PixelDriver);

//Objects for Buttons and Button Macros:
PIN_DRIVER ModePin(11);
PIN_MACRO ModeMacro;

//4 Button Pins, 3 for Pedal(4,3,2), 1(7) for External:
PIN_DRIVER ButtonPin[] = {(4), (3), (2), (7)};
//16(4-Bit) Macros for Dynamic Mapping:
PIN_MACRO ButtonMacro[16];

//Makes it a little easier:
#define CONFIG_BUFF(ADD) RTX_PROTOCAL.RTX_EEPROM_BUFFER[ADD]
#define DEBUG_BUFF(ADD) RTX_PROTOCAL.RTX_DEBUG_BUFFER[ADD]

//Variable for Button Macro Mode
uint8_t Mode = 0;

uint8_t COMP_CONNECTION = 0;
bool ATEM_CONNECTION = false;
bool RTX_LINK = false; //If Connected to RTX BUS and Ether IP Linked
uint32_t RTX_LINK_TIMER = 0;

uint8_t ATEM_TALLY_PRG = 0;
uint8_t ATEM_TALLY_PRE = 0;

void setup()
{
  PixelDriver.InitPixel();
  for (uint8_t X = 0; X < 3; X++)
  {
    StatusMacros[X].SetFPS(60);
    TallyMacros[X].SetFPS(60);
  }
}

void loop()
{
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR)
  {
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(1) = ATEM_TALLY_PRG; //PRG
      DEBUG_BUFF(2) = ATEM_TALLY_PRE; //PRE
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
      if (RTX_PROTOCAL.Read(IP, Bytes, MSG))
      {
        if (RTX_PROTOCAL.RTX_MSG_TYPE[0] == 2 || RTX_PROTOCAL.RTX_MSG_TYPE[0] == 3)
        {
          //Verify if the Message is comming from Ethernet:
          if (IP == (CONFIG_BUFF(199) & B00000111))
          {
            if (Bytes == 3 && !(MSG[0] & B00011100))
            {
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
      if (TimerHold && RTX_LINK)
      {
        if (millis() - RTX_LINK_TIMER >= TimerHold)
        {
          RTX_LINK = false;
          COMP_CONNECTION = 0;
          ATEM_CONNECTION = false;
        }
      }
    }
    //Only Run Buttons if Not EEPROM ERROR:
    BUTTON_RUN();
  }
  else
  {
    RTX_LINK = false;
    COMP_CONNECTION = 0;
    ATEM_CONNECTION = false;
    //Update Tally Debug Registers:
    {
      DEBUG_BUFF(1) = ATEM_TALLY_PRG; //PRG
      DEBUG_BUFF(2) = ATEM_TALLY_PRE; //PRE
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
  LED_RUN();
}

void BUTTON_RUN()
{
  //Mode Button Run:
  {
    static bool ModeSet = true;
    if (ModeMacro.Run(ModePin.Run()))
    {
      if (!ModeMacro.State())
      {
        ModeSet = false;
      }
      else
      {
        if (!ModeSet)
        {
          Mode++;
          Mode &= B00000011;
        }
      }
    }
    if (!ModeSet && !ModeMacro.State() && ModeMacro.Interval() >= 300)
    {
      ModeSet = true;
      Mode = 0;
    }
  }
  //Multi Function Button Run:
  {
    //Static Bool For Keepting Track if Macro Ran Delay Yet:
    static bool DelayMacroRun[16] = {
        false, false, false, false,
        false, false, false, false,
        false, false, false, false,
        false, false, false, false};
    //Run and Gather Interrupt Data from all 16 Pin Macros
    //Put all Interrupt Data in Here:
    //Byte(Bit 7-0) [0,0,0,LONG,SHORT,DELAY,OFF(StateChange),ON(StateChange)]
    uint8_t PinMacroInterrupt[16] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    //Run Pin Drivers First:
    for (uint8_t X = 0; X < 4; X++)
    {
      ButtonPin[X].Run();
    }
    for (uint8_t X = 0; X < 16; X++)
    {
      if (ButtonMacro[X].Run(RegButtonState(CONFIG_BUFF((8 * X) + 6))))
      {
        if (ButtonMacro[X].State())
        {
          PinMacroInterrupt[X] |= B00000001;
        }
        else
        {
          PinMacroInterrupt[X] |= B00000010;
          if (ButtonMacro[X].PrevInterval() >=
              (CONFIG_BUFF((((CONFIG_BUFF((8 * X) + 6) & B00110000) >> 4) * 8) + 135) | (CONFIG_BUFF((((CONFIG_BUFF((8 * X) + 6) & B00110000) >> 4) * 8) + 134) << 8)))
          {
            PinMacroInterrupt[X] |= B00010000;
          }
          else
          {
            PinMacroInterrupt[X] |= B00001000;
          }
          DelayMacroRun[X] = false;
        }
      }
      if (!DelayMacroRun[X] && ButtonMacro[X].State() && ButtonMacro[X].Interval() >= (CONFIG_BUFF((((CONFIG_BUFF((8 * X) + 6) & B00110000) >> 4) * 8) + 135) | (CONFIG_BUFF((((CONFIG_BUFF((8 * X) + 6) & B00110000) >> 4) * 8) + 134) << 8)))
      {
        PinMacroInterrupt[X] |= B00000100;
        DelayMacroRun[X] = true;
      }
    }

    //Run Through All the Macros:
    for (uint8_t X = 0; X < 20; X++)
    {
      if ((1 << (Mode & B00000011)) & CONFIG_BUFF((8 * X)) && PinMacroInterrupt[(CONFIG_BUFF((8 * X)) & B11110000) >> 4])
      {
        for (uint8_t Y = 0; Y < 3; Y++)
        {
          if ((CONFIG_BUFF((8 * X) + 1 + Y) & B11100000) >> 5 > 0 && (CONFIG_BUFF((8 * X) + 1 + Y) & B11100000) >> 5 < 6)
          {
            if (PinMacroInterrupt[(CONFIG_BUFF((8 * X)) & B11110000) >> 4] & (1 << ((CONFIG_BUFF((8 * X) + 1 + Y) & B11100000) >> 5) - 1))
              RunMacro(CONFIG_BUFF(((CONFIG_BUFF((8 * X) + 1 + Y) & B00001111) * 8) + 7));
          }
        }
      }
    }
  }
}

bool RegButtonState(uint8_t BTN_CMD)
{
  //Check if at least one of the buttons is selected:
  if (BTN_CMD & B00001111)
  {
    bool BTN_Trigger = (BTN_CMD & B01000000);
    if (BTN_CMD & B10000000)
    {
      //AND
      bool ButtonAND = true;
      for (uint8_t X = 0; X < 4 && ButtonAND; X++)
      {
        if ((BTN_CMD & (1 << X)))
        {
          if (ButtonPin[X].ButtonState() != BTN_Trigger)
            ButtonAND = false;
        }
      }
      if (ButtonAND)
        return true;
    }
    else
    {
      //OR
      for (uint8_t X = 0; X < 4; X++)
      {
        if ((BTN_CMD & (1 << X)))
        {
          if (ButtonPin[X].ButtonState() == BTN_Trigger)
            return true;
        }
      }
    }
  }
  return false;
}

void RunMacro(uint8_t Macro)
{
  const uint16_t TimeOut = 500;
  static uint32_t ButtonTimeOut = (millis() - TimeOut);
  switch (((Macro & B11000000) >> 6))
  {
  case 0:
    //No Macro, Empty
    break;
  case 1:
  {
    //Companion
    uint8_t TX_MSG[3] = {B00100011, (CONFIG_BUFF(192) & B01111111), (Macro & B00011111)};
    if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
      DEBUG_BUFF(15)
    ++;
    else DEBUG_BUFF(14)++;
  }
  break;
  case 2:
    //ATEM
    break;
  case 3:
    //Custom
    switch ((Macro & B00111111))
    {
    case 0:
      //Check if One of the Cameras are selected
      if (ATEM_CONNECTION)
      {
        if ((ATEM_TALLY_PRG & B00110000) && (ATEM_TALLY_PRG & B00110000) != B00110000)
        {
          if ((ATEM_TALLY_PRG & B00010000) && !(ATEM_TALLY_PRE & B00100000))
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 28};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
          else if ((ATEM_TALLY_PRG & B00100000) && !(ATEM_TALLY_PRE & B00010000))
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 29};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
        }
      }
      break;
    case 1:
      //Short Cut
      if (millis() - ButtonTimeOut >= TimeOut)
      {
        if (ATEM_CONNECTION)
        {
          if ((ATEM_TALLY_PRG & B00110000))
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 26};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
          else
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 25};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
        }
        else
        {
          uint8_t TX_MSG[3] = {B00100011, 4, 26};
          if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
            DEBUG_BUFF(15)
          ++;
          else DEBUG_BUFF(14)++;
        }
        ButtonTimeOut = millis();
      }
      break;
    case 2:
      //Long Auto
      if (millis() - ButtonTimeOut >= TimeOut)
      {
        if (ATEM_CONNECTION)
        {
          if ((ATEM_TALLY_PRG & B00110000))
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 27};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
          else
          {
            uint8_t TX_MSG[3] = {B00100011, 4, 24};
            if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
              DEBUG_BUFF(15)
            ++;
            else DEBUG_BUFF(14)++;
          }
        }
        else
        {
          uint8_t TX_MSG[3] = {B00100011, 4, 27};
          if (!RTX_PROTOCAL.Write((CONFIG_BUFF(199) & B00000111), 3, TX_MSG))
            DEBUG_BUFF(15)
          ++;
          else DEBUG_BUFF(14)++;
        }
        ButtonTimeOut = millis();
      }
      break;
    case 3:
      break;
    }
    break;
  }
}

void LED_RUN()
{
  {   //Set which macro should be running here:
    { //Set Error Macros:
      if (CONFIG_BUFF(194) == 0)
      {
        uint8_t ErrorMacroMode = 0;
        if (COMP_CONNECTION != 1)
          ErrorMacroMode = 1;
        ErrorMacroMode |= (!ATEM_CONNECTION << 1);
        ErrorBuild.Macro = ErrorMacroMode;
      }
      else
      {
        ErrorBuild.Macro = 0;
      }
    }
    //Set Status Macros
    {
      if (CONFIG_BUFF(194) == 0)
      {
        StatusBuild.Macro = ((Mode % 4) + 1);
      }
      else if (CONFIG_BUFF(194) == 1)
      {
        StatusBuild.Macro = 5;
      }
      else
      {
        StatusBuild.Macro = 6;
      }
    }
    //Set Tally Macros:
    {
      if (CONFIG_BUFF(194) == 0)
      {
        uint8_t TallyStatus = ((bool)(ATEM_TALLY_PRG & CONFIG_BUFF(166)) | ((bool)(ATEM_TALLY_PRE & CONFIG_BUFF(167)) << 1));
        switch (TallyStatus)
        {
        case 0:
          //No Tally
          TallyBuild.Macro = 0;
          break;
        case 1:
          //PRG
          if (CONFIG_BUFF(175) & B00000100)
            TallyBuild.Macro = 1;
          else
            TallyBuild.Macro = 0;
          break;
        case 2:
          //PRE
          if (CONFIG_BUFF(175) & B00001000)
            TallyBuild.Macro = 2;
          else
            TallyBuild.Macro = 0;
          break;
        case 3:
          //PRG/PRE
          TallyBuild.Macro = (CONFIG_BUFF(175) & B00000011);
          break;
        }
      }
      else if (CONFIG_BUFF(194) == 1)
      {
        TallyBuild.Macro = 4;
      }
      else
      {
        TallyBuild.Macro = 5;
      }
    }
    //Set Status Macro Values:
    {
      switch (StatusRed.MacroType)
      {
      case 0:
        StatusRed.Set(StatusMacros[0].Val());
        break;
      case 1:
        StatusRed.Set(ErrorMacro[0].Val());
        break;
      }
      switch (StatusGreen.MacroType)
      {
      case 0:
        StatusGreen.Set(StatusMacros[1].Val());
        break;
      case 1:
        StatusGreen.Set(ErrorMacro[1].Val());
        break;
      }
      switch (StatusBlue.MacroType)
      {
      case 0:
        StatusBlue.Set(StatusMacros[2].Val());
        break;
      case 1:
        StatusBlue.Set(ErrorMacro[2].Val());
        break;
      }
    }
    //Set Tally Values:
    {
      switch (TallyRed.MacroType)
      {
      case 0:
        TallyRed.Set(TallyMacros[0].Val());
        break;
      case 1:
        TallyRed.Set(ErrorMacro[0].Val());
        break;
      }
      switch (TallyGreen.MacroType)
      {
      case 0:
        TallyGreen.Set(TallyMacros[1].Val());
        break;
      case 1:
        TallyGreen.Set(ErrorMacro[1].Val());
        break;
      }
      switch (TallyBlue.MacroType)
      {
      case 0:
        TallyBlue.Set(TallyMacros[2].Val());
        break;
      case 1:
        TallyBlue.Set(ErrorMacro[2].Val());
        break;
      }
    }

    //Write to NEO-PIXELS:
    {
      static uint32_t PixelTimer = millis();
      if (PixelDriver.WritePixel)
      {
        PixelTimer = millis();
      }
      else
      {
        if (millis() - PixelTimer >= 100)
        {
          PixelDriver.WritePixel = true;
        }
      }
      PixelDriver.Write();
    }
  }
  //Status LED Macros:
  switch (StatusBuild.Macro)
  {
  case 0:
    //Led Off not running anything
    if (StatusBuild.MacroChange())
    {
      for (uint8_t X = 0; X < 3; X++)
        StatusMacros[X].Fade(0, 12); //12 Frames, 12/60FPS: 1/5(second)
    }
    StatusMacros[0].Run();
    StatusMacros[1].Run();
    StatusMacros[2].Run();
    break;
  case 1:
    if (StatusBuild.MacroChange())
    {
      StatusMacros[0].RST();
      StatusMacros[1].RST();
      StatusMacros[2].RST();
    }
    if (StatusMacros[0].Run())
      StatusMacros[0].Fade(CONFIG_BUFF(160 + (0 * 8) + 0), 12);
    if (StatusMacros[1].Run())
      StatusMacros[1].Fade(CONFIG_BUFF(160 + (0 * 8) + 1), 12);
    if (StatusMacros[2].Run())
      StatusMacros[2].Fade(CONFIG_BUFF(160 + (0 * 8) + 2), 12);
    break;
  case 2:
    if (StatusBuild.MacroChange())
    {
      StatusMacros[0].RST();
      StatusMacros[1].RST();
      StatusMacros[2].RST();
    }
    if (StatusMacros[0].Run())
      StatusMacros[0].Fade(CONFIG_BUFF(160 + (1 * 8) + 0), 12);
    if (StatusMacros[1].Run())
      StatusMacros[1].Fade(CONFIG_BUFF(160 + (1 * 8) + 1), 12);
    if (StatusMacros[2].Run())
      StatusMacros[2].Fade(CONFIG_BUFF(160 + (1 * 8) + 2), 12);
    break;
  case 3:
    if (StatusBuild.MacroChange())
    {
      StatusMacros[0].RST();
      StatusMacros[1].RST();
      StatusMacros[2].RST();
    }
    if (StatusMacros[0].Run())
      StatusMacros[0].Fade(CONFIG_BUFF(160 + (2 * 8) + 0), 12);
    if (StatusMacros[1].Run())
      StatusMacros[1].Fade(CONFIG_BUFF(160 + (2 * 8) + 1), 12);
    if (StatusMacros[2].Run())
      StatusMacros[2].Fade(CONFIG_BUFF(160 + (2 * 8) + 2), 12);
    break;
  case 4:
    if (StatusBuild.MacroChange())
    {
      StatusMacros[0].RST();
      StatusMacros[1].RST();
      StatusMacros[2].RST();
    }
    if (StatusMacros[0].Run())
      StatusMacros[0].Fade(CONFIG_BUFF(160 + (3 * 8) + 0), 12);
    if (StatusMacros[1].Run())
      StatusMacros[1].Fade(CONFIG_BUFF(160 + (3 * 8) + 1), 12);
    if (StatusMacros[2].Run())
      StatusMacros[2].Fade(CONFIG_BUFF(160 + (3 * 8) + 2), 12);
    break;
  case 5:

    if (StatusBuild.MacroChange())
    {
      StatusBuild.MacroStage = 0;
      StatusMacros[0].Fade(100, 200);
      StatusMacros[1].Fade(100, 200);
      StatusMacros[2].Fade(100, 200);
    }
    StatusMacros[0].Run();
    StatusMacros[1].Run();
    StatusMacros[2].Run();
    if (StatusMacros[0].Ready() && StatusMacros[1].Ready() && StatusMacros[2].Ready())
    {
      switch (StatusBuild.MacroStage)
      {
      case 0:
        StatusBuild.MacroStage = 1;
        StatusMacros[0].Fade(255, 200);
        StatusMacros[1].Fade(255, 200);
        StatusMacros[2].Fade(0, 200);
        break;
      case 1:
        StatusBuild.MacroStage = 2;
        StatusMacros[0].SetDelay(1000);
        StatusMacros[1].SetDelay(1000);
        StatusMacros[2].SetDelay(1000);
        break;
      case 2:
        StatusBuild.MacroStage = 3;
        StatusMacros[0].Fade(255, 200);
        StatusMacros[1].Fade(0, 200);
        StatusMacros[2].Fade(255, 200);
        break;
      case 3:
        StatusBuild.MacroStage = 4;
        StatusMacros[0].SetDelay(1000);
        StatusMacros[1].SetDelay(1000);
        StatusMacros[2].SetDelay(1000);
        break;
      case 4:
        StatusBuild.MacroStage = 5;
        StatusMacros[0].Fade(0, 200);
        StatusMacros[1].Fade(255, 200);
        StatusMacros[2].Fade(255, 200);
        break;
      case 5:
        StatusBuild.MacroStage = 0;
        StatusMacros[0].SetDelay(1000);
        StatusMacros[1].SetDelay(1000);
        StatusMacros[2].SetDelay(1000);
        break;
      }
    }
    break;
  case 6:

    if (StatusBuild.MacroChange())
    {
      StatusBuild.MacroStage = 0;
      StatusMacros[0].Fade(100, 200);
      StatusMacros[1].Fade(100, 200);
      StatusMacros[2].Fade(100, 200);
    }
    StatusMacros[0].Run();
    StatusMacros[1].Run();
    StatusMacros[2].Run();
    if (StatusMacros[0].Ready() && StatusMacros[1].Ready() && StatusMacros[2].Ready())
    {
      switch (StatusBuild.MacroStage)
      {
      case 0:
        StatusBuild.MacroStage = 1;
        StatusMacros[0].Fade(255, 200);
        StatusMacros[1].Fade(0, 200);
        StatusMacros[2].Fade(0, 200);
        break;
      case 1:
        StatusBuild.MacroStage = 2;
        StatusMacros[0].Fade(255, 200);
        StatusMacros[1].Fade(255, 200);
        StatusMacros[2].Fade(0, 200);
        break;
      case 2:
        StatusBuild.MacroStage = 3;
        StatusMacros[0].Fade(0, 200);
        StatusMacros[1].Fade(255, 200);
        StatusMacros[2].Fade(0, 200);
        break;
      case 3:
        StatusBuild.MacroStage = 4;
        StatusMacros[0].Fade(0, 200);
        StatusMacros[1].Fade(255, 200);
        StatusMacros[2].Fade(255, 200);
        break;
      case 4:
        StatusBuild.MacroStage = 5;
        StatusMacros[0].Fade(0, 200);
        StatusMacros[1].Fade(0, 200);
        StatusMacros[2].Fade(255, 200);
        break;
      case 5:
        StatusBuild.MacroStage = 0;
        StatusMacros[0].Fade(255, 200);
        StatusMacros[1].Fade(0, 200);
        StatusMacros[2].Fade(255, 200);
        break;
      }
    }
    break;
  }

  //Tally LED Macros:
  switch (TallyBuild.Macro)
  {
  case 0:
    //Tally Off not running anything
    if (TallyBuild.MacroChange())
    {
      for (uint8_t X = 0; X < 3; X++)
        TallyMacros[X].Fade(0, 6); //12 Frames, 12/60FPS: 1/5(second)
    }
    TallyMacros[0].Run();
    TallyMacros[1].Run();
    TallyMacros[2].Run();
    break;
  case 1:
    //PRG Tally
    if (TallyBuild.MacroChange())
    {
      TallyMacros[0].RST();
      TallyMacros[1].RST();
      TallyMacros[2].RST();
    }
    if (TallyMacros[0].Run())
      TallyMacros[0].Fade(CONFIG_BUFF(163 + (0 * 8) + 0), 6);
    if (TallyMacros[1].Run())
      TallyMacros[1].Fade(CONFIG_BUFF(163 + (0 * 8) + 1), 6);
    if (TallyMacros[2].Run())
      TallyMacros[2].Fade(CONFIG_BUFF(163 + (0 * 8) + 2), 6);
    break;
  case 2:
    //PRE Tally
    if (TallyBuild.MacroChange())
    {
      TallyMacros[0].RST();
      TallyMacros[1].RST();
      TallyMacros[2].RST();
    }
    if (TallyMacros[0].Run())
      TallyMacros[0].Fade(CONFIG_BUFF(163 + (1 * 8) + 0), 6);
    if (TallyMacros[1].Run())
      TallyMacros[1].Fade(CONFIG_BUFF(163 + (1 * 8) + 1), 6);
    if (TallyMacros[2].Run())
      TallyMacros[2].Fade(CONFIG_BUFF(163 + (1 * 8) + 2), 6);
    break;
  case 3:
    //PRG/PRE Mix Tally
    if (TallyBuild.MacroChange())
    {
      TallyMacros[0].RST();
      TallyMacros[1].RST();
      TallyMacros[2].RST();
    }
    if (TallyMacros[0].Run())
      TallyMacros[0].Fade((CONFIG_BUFF(163 + (0 * 8) + 0) + CONFIG_BUFF(163 + (1 * 8) + 0)) / 2, 6);
    if (TallyMacros[1].Run())
      TallyMacros[1].Fade((CONFIG_BUFF(163 + (0 * 8) + 1) + CONFIG_BUFF(163 + (1 * 8) + 1)) / 2, 6);
    if (TallyMacros[2].Run())
      TallyMacros[2].Fade((CONFIG_BUFF(163 + (0 * 8) + 2) + CONFIG_BUFF(163 + (1 * 8) + 2)) / 2, 6);
    break;
  case 4:

    if (TallyBuild.MacroChange())
    {
      TallyBuild.MacroStage = 0;
      TallyMacros[0].Fade(100, 200);
      TallyMacros[1].Fade(100, 200);
      TallyMacros[2].Fade(100, 200);
    }
    TallyMacros[0].Run();
    TallyMacros[1].Run();
    TallyMacros[2].Run();
    if (TallyMacros[0].Ready() && TallyMacros[1].Ready() && TallyMacros[2].Ready())
    {
      switch (TallyBuild.MacroStage)
      {
      case 0:
        TallyBuild.MacroStage = 1;
        TallyMacros[0].Fade(255, 200);
        TallyMacros[1].Fade(255, 200);
        TallyMacros[2].Fade(0, 200);
        break;
      case 1:
        TallyBuild.MacroStage = 2;
        TallyMacros[0].SetDelay(1000);
        TallyMacros[1].SetDelay(1000);
        TallyMacros[2].SetDelay(1000);
        break;
      case 2:
        TallyBuild.MacroStage = 3;
        TallyMacros[0].Fade(255, 200);
        TallyMacros[1].Fade(0, 200);
        TallyMacros[2].Fade(255, 200);
        break;
      case 3:
        TallyBuild.MacroStage = 4;
        TallyMacros[0].SetDelay(1000);
        TallyMacros[1].SetDelay(1000);
        TallyMacros[2].SetDelay(1000);
        break;
      case 4:
        TallyBuild.MacroStage = 5;
        TallyMacros[0].Fade(0, 200);
        TallyMacros[1].Fade(255, 200);
        TallyMacros[2].Fade(255, 200);
        break;
      case 5:
        TallyBuild.MacroStage = 0;
        TallyMacros[0].SetDelay(1000);
        TallyMacros[1].SetDelay(1000);
        TallyMacros[2].SetDelay(1000);
        break;
      }
    }
    break;
  case 5:

    if (TallyBuild.MacroChange())
    {
      TallyBuild.MacroStage = 0;
      TallyMacros[0].Fade(100, 200);
      TallyMacros[1].Fade(100, 200);
      TallyMacros[2].Fade(100, 200);
    }
    TallyMacros[0].Run();
    TallyMacros[1].Run();
    TallyMacros[2].Run();
    if (TallyMacros[0].Ready() && TallyMacros[1].Ready() && TallyMacros[2].Ready())
    {
      switch (TallyBuild.MacroStage)
      {
      case 0:
        TallyBuild.MacroStage = 1;
        TallyMacros[0].Fade(255, 200);
        TallyMacros[1].Fade(0, 200);
        TallyMacros[2].Fade(0, 200);
        break;
      case 1:
        TallyBuild.MacroStage = 2;
        TallyMacros[0].Fade(255, 200);
        TallyMacros[1].Fade(255, 200);
        TallyMacros[2].Fade(0, 200);
        break;
      case 2:
        TallyBuild.MacroStage = 3;
        TallyMacros[0].Fade(0, 200);
        TallyMacros[1].Fade(255, 200);
        TallyMacros[2].Fade(0, 200);
        break;
      case 3:
        TallyBuild.MacroStage = 4;
        TallyMacros[0].Fade(0, 200);
        TallyMacros[1].Fade(255, 200);
        TallyMacros[2].Fade(255, 200);
        break;
      case 4:
        TallyBuild.MacroStage = 5;
        TallyMacros[0].Fade(0, 200);
        TallyMacros[1].Fade(0, 200);
        TallyMacros[2].Fade(255, 200);
        break;
      case 5:
        TallyBuild.MacroStage = 0;
        TallyMacros[0].Fade(255, 200);
        TallyMacros[1].Fade(0, 200);
        TallyMacros[2].Fade(255, 200);
        break;
      }
    }
    break;
  }

  //ERROR Macros
  switch (ErrorBuild.Macro)
  {
  case 0:
    if (ErrorBuild.MacroChange())
    {
      StatusRed.MacroType = 0;
      StatusGreen.MacroType = 0;
      StatusBlue.MacroType = 0;
      TallyRed.MacroType = 0;
      TallyGreen.MacroType = 0;
      TallyBlue.MacroType = 0;
    }
    break;
  case 1:
    //COMP Not Connected
    {
      if (ErrorBuild.MacroChange())
      {
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

      if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready())
      {
        switch (ErrorBuild.MacroStage)
        {
        case 0:
          ErrorMacro[0].Set(0, 50);

          StatusRed.MacroType = 1;
          StatusGreen.MacroType = 1;
          StatusBlue.MacroType = 1;

          TallyRed.MacroType = 1;
          TallyGreen.MacroType = 1;
          TallyBlue.MacroType = 1;

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

          TallyRed.MacroType = 0;
          TallyGreen.MacroType = 0;
          TallyBlue.MacroType = 0;

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
      if (ErrorBuild.MacroChange())
      {
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

      if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready())
      {
        switch (ErrorBuild.MacroStage)
        {
        case 0:
          ErrorMacro[0].Set(0, 50);

          StatusRed.MacroType = 1;
          StatusGreen.MacroType = 1;
          StatusBlue.MacroType = 1;

          TallyRed.MacroType = 1;
          TallyGreen.MacroType = 1;
          TallyBlue.MacroType = 1;

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

          TallyRed.MacroType = 0;
          TallyGreen.MacroType = 0;
          TallyBlue.MacroType = 0;

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
      if (ErrorBuild.MacroChange())
      {
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

      if (ErrorMacro[0].Ready() && ErrorMacro[1].Ready() && ErrorMacro[2].Ready())
      {
        switch (ErrorBuild.MacroStage)
        {
        case 0:
          ErrorMacro[0].Set(0, 50);

          StatusRed.MacroType = 1;
          StatusGreen.MacroType = 1;
          StatusBlue.MacroType = 1;

          TallyRed.MacroType = 1;
          TallyGreen.MacroType = 1;
          TallyBlue.MacroType = 1;

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

          TallyRed.MacroType = 0;
          TallyGreen.MacroType = 0;
          TallyBlue.MacroType = 0;

          ErrorBuild.MacroStage = 0;
          break;
        default:
          ErrorBuild.MacroStage = 0;
          break;
        }
      }
    }
    break;
  case 4:
    //EEPROM ERROR:
    break;
  }
}

uint8_t ModeInc(uint8_t CurrMode)
{
  bool ModeFound = false;
  for (uint8_t X = 0; X < 4 && !ModeFound; X++)
  {
    CurrMode++;
    if (CurrMode > 3)
      CurrMode = 0;
    if (CONFIG_BUFF(193) & (1 << CurrMode))
      ModeFound = true;
  }
  if (ModeFound)
    return CurrMode;
  else
    return 0;
}

bool ModeAvail()
{
  if ((CONFIG_BUFF(193) & B10000000) && (CONFIG_BUFF(193) & B00001111))
    return true;
  else
    return false;
}

uint8_t ModeRST()
{
}

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.0.h"

// other libs to help with example:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\Other\OUTPUT_MACRO\Core\MACRO_BUILD\MACRO_BUILD_1.0.5\MACRO_DRIVER_1.0.1.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\Other\OUTPUT_MACRO\Core\MACRO_BUILD\MACRO_BUILD_1.0.5\BUTTON_INTERFACE.h"

SequenceBuild sequBuild;
MACRO_DRIVER Macro;

PIN_DRIVER Button(3);
PIN_MACRO PinMacro;

void setup()
{
  Serial.begin(115200);
  Serial.println("init");
  sequBuild.setSequence(fade, 2);
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);
  Macro.FPS(60);
}

void loop()
{
  sequBuild.run();
  analogWrite(5, Macro.Val());

  if (PinMacro.Run(Button.Run()) && !Button.ButtonState())
  {
    // static uint8_t sequRunning = 0;
    // if (sequRunning == 0)
    // {
    //   sequRunning = 1;
    //   sequBuild.setSequence(blink, 0, true);
    // }
    // else if (sequRunning == 1)
    {
      // sequRunning = 0;
      sequBuild.setSequence(blink, 0, true);
    }
  }
}

SB_FUNCT(fade, Macro.Run())
SB_STEP(Macro.Set(0, 50);)
SB_STEP(Macro.Fade(100, 10);)
SB_STEP(Macro.Fade(100, 120);)
SB_STEP(Macro.Delay(100);)
SB_STEP(Macro.Fade(0, 120);)
SB_STEP(Macro.Delay(500);)
SB_STEP(sequBuild.loop(2);)
SB_END

SB_FUNCT(blink, Macro.Run())
SB_STEP(Macro.Set(100, 50);)
SB_STEP(Macro.Set(0, 50);)
SB_STEP(Macro.Set(100, 50);)
SB_STEP(Macro.Set(0, 50);)
SB_STEP(Macro.Set(100, 50);)
SB_STEP(Macro.Set(0, 50);)
SB_STEP(Macro.Set(100, 50);)
SB_STEP(Macro.Set(0, 500);)
SB_STEP(sequBuild.setSequence(fade, 0, true);)
SB_END

SB_FUNCT(sequ_1, (PinMacro.Run(Button.Run()) && !Button.ButtonState()))
SB_STEP(Serial.println("index 0");)
SB_STEP(Serial.println("index 1");)
SB_STEP(Serial.println("loop to 0");
        sequBuild.loop(0);)
SB_END

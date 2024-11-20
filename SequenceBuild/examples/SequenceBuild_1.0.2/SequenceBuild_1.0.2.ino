#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.2.h"

// other libs to help with example:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\PinDriver\PinDriver_1.0.1.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\InputMacro\InputMacro_1.0.1.h"

SequenceBuild build;

LedMacro _macros[3];
LedMacroManager macro(_macros, 3);

PinDriver pin(3);
InputMacro pinMacro(true);

uint8_t ledval = 0;

void setup()
{
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);
  build.setSequence(fade, 0, true);

}

void loop()
{
  macro.run();
  build.run();
  analogWrite(5, ledval);

  if (pinMacro(pin))
  {
    if (!pinMacro)
    {
      build.setPrioritySequence(flash, 0, true);
    }
  }
  build.setSequence(fade, 0, true);
}

SB_FUNCT(fade, macro.ready(ledval))
SB_STEP(macro.quadEase(ledval, 100, 15);)
SB_STEP(macro.quadEase(ledval, 0, 15);)
SB_STEP(build.loop(0);)
SB_END

SB_FUNCT(flash, macro.ready(ledval))
SB_STEP(macro.quadEase(ledval, 100, 15);)
SB_STEP(macro.delay(ledval, 1000);)
SB_STEP(macro.quadEase(ledval, 0, 15);)
SB_END

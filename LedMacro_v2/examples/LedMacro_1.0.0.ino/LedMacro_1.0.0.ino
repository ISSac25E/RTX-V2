#include "arduino.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.0.h"

LedMacro _macros[2];
LedMacroManager macro(_macros, 2);

SequenceBuild LedBuild_1;
SequenceBuild LedBuild_2;

uint8_t led_1 = 0;
uint8_t led_2 = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("led_1,led2");
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);

  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  LedBuild_1.setSequence(blink, 2);
  LedBuild_2.setSequence(fade, 2);
}
void loop()
{
  macro.run();
  LedBuild_1.run();
  LedBuild_2.run();
  printval_timed();

  analogWrite(5, led_1);
  analogWrite(6, led_2);
}

void printval_timed()
{
  static uint32_t timer = millis();
  if (millis() - timer >= 16)
  {
    timer = millis();
    Serial.print(led_1);
    Serial.print(",");
    Serial.println(led_2);
  }
}

SB_FUNCT(blink, macro.ready(led_1))
SB_STEP(macro.set(led_1, 100, 50);)
SB_STEP(macro.set(led_1, 0, 50);)
SB_STEP(macro.set(led_1, 100, 50);)
SB_STEP(macro.set(led_1, 0, 50);)
SB_STEP(macro.set(led_1, 100, 50);)
SB_STEP(macro.set(led_1, 0, 50);)
SB_STEP(macro.set(led_1, 100, 50);)
SB_STEP(macro.set(led_1, 0, 50);)
SB_STEP(macro.set(led_1, 0, 500);)
SB_STEP(LedBuild_1.loop(0);)
SB_END

SB_FUNCT(fade, macro.ready(led_2))
SB_STEP(macro.set(led_2, 0, 50);)
SB_STEP(macro.quadEase(led_2, 100, 10);)
SB_STEP(macro.quadEase(led_2, 100, 20);)
// SB_STEP(macro.delay(led_2, 100);)
SB_STEP(macro.quadEase(led_2, 0, 20);)
SB_STEP(macro.delay(led_2, 100);)
SB_STEP(LedBuild_2.loop(2);)
SB_END
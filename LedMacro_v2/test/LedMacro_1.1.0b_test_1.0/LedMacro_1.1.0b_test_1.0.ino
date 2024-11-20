#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\Other\IR RGB Controller\Core\LedMacro\LedMacro_1.1.0b.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\Other\IR RGB Controller\Core\SequenceBuild\SequenceBuild_1.0.4.h"

LedMacro _macro[5];
LedMacroManager macro(_macro, sizeof(_macro) / sizeof(_macro[0]));

SequenceBuild ledBuild;

uint8_t whiteVal = 255;

#define WhiteLed_Pin 6

void setup()
{
  Serial.begin(115200);

  macro.fps(240);
  digitalWrite(WhiteLed_Pin, HIGH);
  pinMode(WhiteLed_Pin, OUTPUT);
  // analogWrite(WhiteLed_Pin, whiteVal);
  ledBuild.setSequence(smooth, 0, true);
}

float modX = 1.0;
void loop()
{
  {
    static uint32_t timer = millis();
    if (millis() - timer >= 30)
    {
      timer = millis();
      modX = mapfloat(analogRead(A0), 0, 1023, 0.1, 100);
      Serial.println(modX);
      macro.mod(whiteVal, modX);
    }
  }
  macro.run();
  ledBuild.run();

  analogWrite(WhiteLed_Pin, whiteVal);
}

SB_FUNCT(smooth, macro.ready(whiteVal))
SB_STEP(macro.quadEase(whiteVal, 0, 1000);macro.mod(whiteVal, modX);)
SB_STEP(macro.delay(whiteVal, 300);macro.mod(whiteVal, modX);)
SB_STEP(macro.quadEase(whiteVal, 255, 1000);macro.mod(whiteVal, modX);)
SB_STEP(macro.delay(whiteVal, 300);macro.mod(whiteVal, modX);)
SB_STEP(_this->loop(0);)
SB_END

float mapfloat(long x, long in_min, long in_max, float out_min, float out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}
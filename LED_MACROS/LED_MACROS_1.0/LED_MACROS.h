//LED_MACROS REV 1.0.0
//.h
#ifndef LED_MACROS_h
#define LED_MACROS_h

#include "Arduino.h"

class LED_MACROS {
  public:

    bool SetPin(uint8_t Pin);

    void Blink();
    void Pulse();
    void Set(uint8_t PWM);
    uint8_t PWM();
    void SetFrameRate(uint8_t FPS);

    bool Run();

  private:

    bool _PinBoot = false;
    uint8_t _Pin;

    uint8_t _PWM;

    //0 = Set, 1 = Blink, 2 = Pulse
    uint8_t _PinMacroState = 0;
};

//.cpp
//#include "LED_MACROS.h"
//#include "Arduino.h"

void LED_MACROS::Pulse() {

}

void LED_MACROS::SetFrameRate(uint8_t FPS) {
  if (FPS = 0) {
    //set it to 0
  }
  else {
    // set it to 1000/FPS
  }
}

bool LED_MACROS::SetPin(uint8_t Pin) {
  //Check if PWM Pin, Only PWM Pins allowed:
  if (Pin == 3 || Pin == 5 || Pin == 6 || Pin == 9 || Pin == 10 || Pin == 11) {
    //Set up pin and vals:
    _Pin = Pin;
    _PinBoot = true;
    analogWrite(_Pin, 0);
    pinMode(_Pin, OUTPUT);
    _PWM = 0;
    return true;
  }
  //Not PWM Pin, wont work
  _PinBoot = false;
  return false;
}

void LED_MACROS::Set(uint8_t PWM) {
  if (_PinBoot) {
    _PinMacroState = 0;
    _PWM = PWM;
    analogWrite(_Pin, _PWM);
  }
}

uint8_t LED_MACROSP::PWM() {
  return _PWM;
}

#endif 

//LED_MACROS REV 2.0.0
//.h
#ifndef LED_MACROS_h
#define LED_MACROS_h

#include "Arduino.h"

class LED_MACROS {
  public:
    bool Run();
    bool SetPin(uint8_t Pin);
    void Fade(uint8_t Target, uint8_t Frames);
    void Set(uint8_t Target, uint16_t Delay);
    void Set(uint16_t Delay);
    void SetFPS(uint16_t FPS);
    uint8_t PWM() {
      return _PWM;
    }
    uint8_t Last() {
      return _MacroState;
    }

  private:
    uint8_t _Pin;
    bool _PinBoot = false;
    bool _MacroRun;

    //Which Macro is Running: 0 = Set Low, 1 = Set High, 2 = Fade Low, 3 = Fade High
    uint8_t _MacroState;
    uint32_t _Timer;
    uint16_t _Delay;

    //Rate For Frames. Default is 33ms for 30 FPS:
    uint16_t _Rate = 33;
    uint8_t _PWM;
    uint8_t _Target;
    uint8_t _Increment;
};

//.cpp
//#include "LED_MACROS.h"
//#include "Arduino.h"

void LED_MACROS::SetFPS(uint16_t FPS) {
  if(FPS) {
    if(FPS > 1000) {
      _Rate = 1;
    }
    else {
      _Rate = 1000/FPS;
    }
  }
  else {
    _Rate = 0;
  }
}

void LED_MACROS::Set(uint16_t Delay) {
  if (_PinBoot) {
    if (Delay) {
      _Delay = Delay;
      _Timer = millis();
      _MacroRun = true;
    }
    else {
      _MacroRun = false;
    }
    analogWrite(_Pin, _PWM);
    if (_MacroState == 0 || _MacroState == 2) _MacroState = 0;
    else _MacroState = 1;
  }
}

void LED_MACROS::Set(uint8_t Target, uint16_t Delay) {
  if (_PinBoot) {
    if (Delay) {
      _Delay = Delay;
      _Timer = millis();
      _MacroRun = true;
      _PWM = Target;
    }
    else {
      _MacroRun = false;
      _PWM = Target;
    }
    analogWrite(_Pin, _PWM);
    if (Target > _PWM) _MacroState = 1;
    else if (Target < _PWM) _MacroState = 0;
    else {
      if (_MacroState == 0 || _MacroState == 2) _MacroState = 0;
      else _MacroState = 1;
    }
  }
}

void LED_MACROS::Fade(uint8_t Target, uint8_t Frames) {
  if (_PinBoot) {
    if (Frames) {
      if (Target > _PWM) {
        _MacroRun = true;
        _MacroState = 3;
        _Delay = _Rate;
        _Increment = (Target - _PWM) / Frames;
        if (!_Increment) _Increment = 1;
        _Target = Target;
        _Timer = millis();
      }
      else if (Target < _PWM) {
        _MacroRun = true;
        _MacroState = 2;
        _Delay = _Rate;
        _Increment = (_PWM - Target) / Frames;
        if (!_Increment) _Increment = 1;
        _Target = Target;
        _Timer = millis();
      }
      else {
        _MacroRun = false;
      }
    }
  }
}

bool LED_MACROS::Run() {
  if (_PinBoot) {
    if (_MacroRun) {
      if ((millis() - _Timer) >= _Delay) {
        if (_MacroState == 2 || _MacroState == 3) {
          //Fade
          if (_PWM < _Target) {
            //Fading Up
            uint8_t PWM_Hold = _PWM + _Increment;
            if (PWM_Hold >= _Target || PWM_Hold <= _PWM) {
              // Done Fading
              PWM_Hold = _Target;
              _MacroRun = false;
            }
            _PWM = PWM_Hold;
          }
          else if (_PWM > _Target) {
            //Fading Down
            uint8_t PWM_Hold = _PWM - _Increment;
            if (PWM_Hold <= _Target || PWM_Hold >= _PWM) {
              // Done Fading
              PWM_Hold = _Target;
              _MacroRun = false;
            }
            _PWM = PWM_Hold;
          }
          else {
            //Finished, not fading anymore
            _MacroRun = false;
          }
          analogWrite(_Pin, _PWM);
        }
        else {
          //Set
          _MacroRun = false;
        }
        _Timer = millis();
      }
    }
    return !_MacroRun;
  }
  return false;
}

bool LED_MACROS::SetPin(uint8_t Pin) {
  if (Pin == 3 || Pin == 5 || Pin == 6 || Pin == 9 || Pin == 10 || Pin == 11) {
    //SetUp all Values here:
    _PinBoot = true;
    _PWM = 0;
    _MacroRun = false;
    _MacroState = 0;
    _Timer = millis();
    //
    _Pin = Pin;
    analogWrite(_Pin, _PWM);
    pinMode(_Pin, OUTPUT);
    return true;
  }
  _PinBoot = false;
  return false;
}
#endif

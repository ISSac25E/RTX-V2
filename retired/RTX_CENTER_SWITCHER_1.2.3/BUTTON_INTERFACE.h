//BUTTON_INTERFACE REV 1.0.3
//.h
#ifndef BUTTON_INTERFACE_h
#define BUTTON_INTERFACE_h
#include "Arduino.h"

class PIN_DRIVER {
  public:
    PIN_DRIVER(uint8_t Pin, bool PullUpMode = true);
    //Returns Pin State and runs Button:
    bool Run();
    //Returns button State, does not run Button:
    bool ButtonState() {
      return _ButtonState;
    }
    void ButtonDebounce(uint16_t Debounce_us) {
      _ButtonDeBounceDelay = Debounce_us;
    }
  private:
    //Pointers to Port Registers
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    uint8_t _PinMask;
    uint8_t _PinMaskNot;  // << Only used if we were to write to Pin, Extra in this case
    //Other Variables for Testing the Button Pin
    bool _ButtonState = false;
    bool _ButtonTest = false;
    uint32_t _ButtonTestTimer = 0;
    uint16_t _ButtonDeBounceDelay = 5000; //Default Value 5ms, 5000us
    //Function for port reading
    bool _PinRead() {
      return (*_PinPort_PIN & _PinMask);
    }
};

class PIN_MACRO {
  public:
    //Returns true if StateChange
    bool Run(bool PinState);
    bool State() {
      return _MacroState;
    }
    bool StateChange(){
      return _StateChange;
    }
    uint32_t PrevInterval() {
      return _MacroPrevInterval;
    }
    uint32_t Interval() {
      return millis() - _MacroIntervalTimer;
    }
    void TimerReset() {
      _MacroIntervalTimer = millis();
    }
    void TimerSet(uint32_t TimerSet) {
      _MacroIntervalTimer = (millis() + TimerSet);
    }
  private:
    //Variables for Running Macro:
    bool _MacroState = false;  //This is for Input PullUps
    bool _StateChange = false;  //This is to tell Weather or not the State Change On the Previous Run
    uint32_t _MacroIntervalTimer = 0;
    uint32_t _MacroPrevInterval = 0;
};

//.cpp
//#include "BUTTON_INTERFACE.h"
//#include "Arduino.h"

PIN_DRIVER::PIN_DRIVER(uint8_t Pin, bool PullUpMode = true) {
  //Pin cannot be larger than 19:
  Pin %= 20;
  //Convert the Pin to Port registers
  if (Pin <= 7) {
    _PinPort_PIN = &PIND;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _PinMask = (1 << Pin);
    _PinMaskNot = ~_PinMask;
  }
  else if (Pin <= 13) {
    _PinPort_PIN = &PINB;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _PinMask = (1 << (Pin - 8));
    _PinMaskNot = ~_PinMask;
    //Port B
  }
  else {
    _PinPort_PIN = &PINC;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _PinMask = (1 << (Pin - 14));
    _PinMaskNot = ~_PinMask;
    //Port C
  }
  //Set Pin to Input Pullup
  *_PinPort_DDR &= _PinMaskNot;
  if (PullUpMode)
    *_PinPort_PORT |= _PinMask;
  else
    *_PinPort_PORT &= _PinMaskNot;
  _ButtonState = _PinRead();
}

bool PIN_DRIVER::Run() {
  if (_ButtonTest) {
    if (micros() - _ButtonTestTimer >= _ButtonDeBounceDelay) {
      if (_PinRead() != _ButtonState) {
        _ButtonState = !_ButtonState;
        _ButtonTest = false;
      }
      else {
        _ButtonTest = false;
      }
    }
  }
  else {
    if (_PinRead() != _ButtonState) {
      _ButtonTest = true;
      _ButtonTestTimer = micros();
    }
  }
  return _ButtonState;
}

bool PIN_MACRO::Run(bool PinState) {
  if(PinState != _MacroState) {
    //State Change:
    _MacroState = PinState;
    _MacroPrevInterval = millis() - _MacroIntervalTimer;
    _MacroIntervalTimer = millis();
    _StateChange = true;
  }
  else _StateChange = false;
  return _StateChange;
}
#endif

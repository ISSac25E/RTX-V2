// BUTTON_INTERFACE REV 1.1.0
//.h
/*
  Use and modify upgraded pinDriver code from wordOfLife projects
*/

/*
purpose of this lib is to use port registers to read from pinInputs
this lib also handles debounce and delays the start state of the button
to prevent false positives from brown-outs
*/

#ifndef PinDriver_h
#define PinDriver_h

#ifndef PinDriverBootDelay_ms
#define PinDriverBootDelay_ms 1000 // define how long button should wait before reading inputs
#endif

#ifndef PinDriverDeBounceDelay_ms
#define PinDriverDeBounceDelay_ms 5 // define debounce delay. 10ms is a good middle ground(glitchy buttons will need longer delay)
#endif

#include "Arduino.h"

class PIN_DRIVER
{
public:
  PIN_DRIVER(uint8_t, bool); // set pin and pullupMode. input(pinNumber, pullupMode(default = true))

  operator bool();    // same as "Run()"
  bool Run();         // Run button debounce and startup delay. returns button state
  bool ButtonState(); // return button state without running button

private:
  bool _pinRead(); // return button state at given point using port read

  bool _pinBoot = false; // used for button cooldown during bootUp
  bool _buttonState;     // keep track of buttonState

  volatile uint8_t *_pinPort_PIN;  // Pin read/state register
  volatile uint8_t *_pinPort_DDR;  // Pin set register (INPUT/OUTPUT)
  volatile uint8_t *_pinPort_PORT; // Pin write register (HIGH/LOW)

  uint8_t _pinMask;    // eg. B00100000
  uint8_t _pinMaskNot; // eg. B11011111

  bool _pinTest = false;         // keep track of when pin is testing
  uint32_t _pinTimer = micros(); // use time to wait for boot up and testing the pin
};

class PIN_MACRO
{
public:
  PIN_MACRO(bool StartState = true)
  {
    _MacroState = StartState;
    this->TimerReset();
  }
  // Returns true if StateChange
  bool Run(bool PinState);
  bool State()
  {
    return _MacroState;
  }
  bool StateChange()
  {
    return _StateChange;
  }
  uint32_t PrevInterval()
  {
    return _MacroPrevInterval;
  }
  uint32_t Interval()
  {
    return millis() - _MacroIntervalTimer;
  }
  void TimerReset()
  {
    _MacroIntervalTimer = millis();
  }
  void TimerSet(uint32_t TimerSet)
  {
    _MacroIntervalTimer = (millis() + TimerSet);
  }

private:
  // Variables for Running Macro:
  bool _MacroState = false;  // This is for Input PullUps
  bool _StateChange = false; // This is to tell Weather or not the State Change On the Previous Run
  uint32_t _MacroIntervalTimer = 0;
  uint32_t _MacroPrevInterval = 0;
};

//.cpp

PIN_DRIVER::PIN_DRIVER(uint8_t pin, bool pullupMode = true)
{
  // set pin registers:
  pin %= 20; // make sure pin does not go out of bounds
  if (pin <= 7)
  {
    // DigitalPins (0 - 7)
    // Port D
    _pinPort_PIN = &PIND;
    _pinPort_DDR = (_pinPort_PIN + 1);
    _pinPort_PORT = (_pinPort_PIN + 2);
    _pinMask = (1 << pin);
    _pinMaskNot = ~_pinMask;
  }
  else if (pin <= 13)
  {
    // DigitalPins (8 - 13)
    // Port B
    _pinPort_PIN = &PINB;
    _pinPort_DDR = (_pinPort_PIN + 1);
    _pinPort_PORT = (_pinPort_PIN + 2);
    _pinMask = (1 << (pin - 8));
    _pinMaskNot = ~_pinMask;
  }
  else
  {
    // Analog Pins (A0 - A7)
    // Port C
    _pinPort_PIN = &PINC;
    _pinPort_DDR = (_pinPort_PIN + 1);
    _pinPort_PORT = (_pinPort_PIN + 2);
    _pinMask = (1 << (pin - 14));
    _pinMaskNot = ~_pinMask;
  }

  *_pinPort_DDR &= _pinMaskNot; // set pin to input
  if (pullupMode)
  {
    *_pinPort_PORT |= _pinMask; // set to pullup
    _buttonState = true;        // set start value
  }
  else
  {
    *_pinPort_PORT = _pinMaskNot; // set to floating pin
    _buttonState = false;         // set start value
  }
}

PIN_DRIVER::operator bool()
{
  return Run();
}

bool PIN_DRIVER::Run()
{
  if (_pinBoot)
  {
    if (_pinTest)
    {
      if (micros() - _pinTimer >= ((uint32_t)PinDriverDeBounceDelay_ms * (uint32_t)1000))
      {
        if (_pinRead() != _buttonState)
          _buttonState = !_buttonState;
        _pinTest = false;
      }
    }
    else if (_pinRead() != _buttonState)
    {
      _pinTest = true;
      _pinTimer = micros();
    }
  }
  else
  {
    if (micros() - _pinTimer >= ((uint32_t)PinDriverBootDelay_ms * (uint32_t)1000))
      _pinBoot = true;
  }
  return _buttonState;
}
bool PIN_DRIVER::ButtonState()
{
  return _buttonState;
}

bool PIN_DRIVER::_pinRead()
{
  return (*_pinPort_PIN & _pinMask);
}

bool PIN_MACRO::Run(bool PinState)
{
  if (PinState != _MacroState)
  {
    //State Change:
    _MacroState = PinState;
    _MacroPrevInterval = millis() - _MacroIntervalTimer;
    _MacroIntervalTimer = millis();
    _StateChange = true;
  }
  else
    _StateChange = false;
  return _StateChange;
}

#endif
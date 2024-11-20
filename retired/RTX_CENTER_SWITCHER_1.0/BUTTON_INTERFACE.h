//BUTTON_INTERFACE REV 1.0.0
//.h
#ifndef BUTTON_INTERFACE_h
#define BUTTON_INTERFACE_h
#include "Arduino.h"

class BUTTON_INTERFACE {
  public:
    BUTTON_INTERFACE(uint8_t Pin, void(*FunctPointer)(), bool PullUpMode = true);
    void Run();
    void ButtonDebounce(uint16_t Debounce_us) {
      _ButtonDeBounceDelay = Debounce_us;
    }
    bool ButtonState() {
      return _ButtonState;
    }
    uint32_t ButtonPrevInterval() {
      return _ButtonPrevInterval;
    }
    uint32_t ButtonCurrInterval() {
      return millis() - _ButtonIntervalTimer;
    }
    void ButtonTimerReset() {
      _ButtonIntervalTimer = millis();
    }


  private:
    //Pointers to Port Registers
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    uint8_t _PinMask;
    uint8_t _PinMaskNot;

    //Variables for Running the Button:
    bool _ButtonState = false;
    bool _ButtonTest = false;
    uint32_t _ButtonIntervalTimer = 0;
    uint32_t _ButtonPrevInterval = 0;
    uint32_t _ButtonTestTimer = 0;
    uint16_t _ButtonDeBounceDelay = 5000; //Default Value 5ms, 5000us

    //Function for port reading
    bool _PinRead() {
      return (*_PinPort_PIN & _PinMask);
    }
    //Pointer for Button Change Interupt Function
    void (*_ButtonChangeFunct)();
};

//.cpp
//#include "BUTTON_INTERFACE.h"
//#include "Arduino.h"

BUTTON_INTERFACE::BUTTON_INTERFACE(uint8_t Pin, void(*FunctPointer)(), bool PullUpMode = true) {
  //Set Function Pointer
  _ButtonChangeFunct = FunctPointer;
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
  if(PullUpMode)
  *_PinPort_PORT |= _PinMask;
  else 
  *_PinPort_PORT &= _PinMaskNot;
  _ButtonState = _PinRead();
  _ButtonIntervalTimer = millis();
}

void BUTTON_INTERFACE::Run() {
  if (_ButtonTest) {
    if (micros() - _ButtonTestTimer >= _ButtonDeBounceDelay) {
      if (_PinRead() != _ButtonState) {
        _ButtonState = !_ButtonState;
        _ButtonPrevInterval = millis() - _ButtonIntervalTimer;
        _ButtonIntervalTimer = millis();
        _ButtonChangeFunct();
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
}

#endif

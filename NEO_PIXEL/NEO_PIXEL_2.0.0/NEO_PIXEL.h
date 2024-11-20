//NEO_PIXEL REV 2.0.0
//.h
#ifndef NEO_PIXEL_h
#define NEO_PIXEL_h
#include "Arduino.h"

////This is how we Write Out the Data to the Neo Pixels
#define BIT_WRITE_LOW(PORT, LO, HI) asm volatile ( \
    "st %a0, %2 \n\t" \
    ".rept 5 \n\t" \
    "nop \n\t" \
    ".endr \n\t" \
    "st %a0, %1 \n\t" \
    ".rept 13 \n\t" \
    "nop \n\t" \
    ".endr \n" \
    :: \
    "e" (PORT), \
    "r" (LO), \
    "r" (HI) \
                                                 );

#define BIT_WRITE_HIGH(PORT, LO, HI) asm volatile ( \
    "st %a0, %2 \n\t" \
    ".rept 9 \n\t" \
    "nop \n\t" \
    ".endr \n\t" \
    "st %a0, %1 \n\t" \
    ".rept 5 \n\t" \
    "nop \n\t" \
    ".endr \n" \
    :: \
    "e" (PORT), \
    "r" (LO), \
    "r" (HI) \
                                                  );
class NEO_PIXEL_DRIVER {
  public:
    NEO_PIXEL_DRIVER(uint8_t Pin);
    uint8_t AddPixel(uint8_t AddSkip);
    bool InitPixel();
    bool IsInit() {
      return _Init;
    }
    uint8_t *PixelValue;
    void Write();

  private:
    //Vars for Pin Port Manipulation:
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    volatile uint8_t _BitMask;
    volatile uint8_t _BitMaskNot;

    //This is for malloc:
    bool _Init = false;
    //This is to keep count of Pixels before Initializing them:
    uint8_t _PixelCount = 0;
    //Used to make sure we don't send two frames to close to each otehr:
    uint32_t _FrameCoolDownTimer = micros();
};

class NEO_PIXEL {
  public:
    //Give it the Pointer to The NeoPixel Led Driver:
    NEO_PIXEL(NEO_PIXEL_DRIVER * NEO_PIXEL_Point, uint8_t AddSkip = 0) {
      _NEO_PIXEL_Point = NEO_PIXEL_Point;
      _PixelAdd = _NEO_PIXEL_Point->AddPixel(AddSkip);
    }
    void Write(uint8_t Val);
    uint8_t Val();
  private:
    //Pointer to NeoPixel Driver Class/Object
    NEO_PIXEL_DRIVER *_NEO_PIXEL_Point;
    //This is the address of our Pixel
    uint8_t _PixelAdd;

};

//.cpp
//#include "NEO_PIXEL.h"
//#include "Arduino.h"

void NEO_PIXEL::Write(uint8_t Val) {
  if(_NEO_PIXEL_Point->PixelValue[_PixelAdd] != Val) {
  _NEO_PIXEL_Point->PixelValue[_PixelAdd] = Val;
  _NEO_PIXEL_Point->Write();
  }
}

uint8_t NEO_PIXEL::Val() {
  return _NEO_PIXEL_Point->PixelValue[_PixelAdd];
}

void NEO_PIXEL_DRIVER::Write() {
  if (_Init) {
    while (micros() - _FrameCoolDownTimer < 100);
    noInterrupts();
    uint8_t Low = (*_PinPort_PORT & _BitMaskNot);
    uint8_t High = (*_PinPort_PORT | _BitMask);
    for (uint8_t X = 0; X < _PixelCount; X++) {
      for (uint8_t Y = 0; Y < 8; Y++) {
        if (bitRead(PixelValue[X],7 - Y)) {
//          Serial.print(1);
          BIT_WRITE_HIGH(_PinPort_PORT, Low, High);
        }
        else {
//          Serial.print(0);
          BIT_WRITE_LOW(_PinPort_PORT, Low, High);
        }
      }
    }
    interrupts();
    _FrameCoolDownTimer = micros();
  }
}

bool NEO_PIXEL_DRIVER::InitPixel() {
  PixelValue = (uint8_t*)malloc(_PixelCount);
  if (PixelValue){
    _Init = true;
    for(uint8_t X = 0; X < _PixelCount; X++) PixelValue[X] = 0;
  }
  else _Init = false;
}

NEO_PIXEL_DRIVER::NEO_PIXEL_DRIVER(uint8_t Pin) {
  Pin %= 20;
  if (Pin <= 7) {
    _PinPort_PIN = &PIND;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _BitMask = (1 << (Pin));
    _BitMaskNot = ~_BitMask;
    //Port D
  }
  else if (Pin <= 13) {
    _PinPort_PIN = &PINB;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _BitMask = (1 << (Pin - 8));
    _BitMaskNot = ~_BitMask;
    //Port B
  }
  else {
    _PinPort_PIN = &PINC;
    _PinPort_DDR = (_PinPort_PIN + 1);
    _PinPort_PORT = (_PinPort_PIN + 2);
    _BitMask = (1 << (Pin - 14));
    _BitMaskNot = ~_BitMask;
    //Port C
  }
  asm volatile (
    "st %a1, %3 \n\t"
    "st %a0, %2\n"
    ::
    "e" (_PinPort_DDR), \
    "e" (_PinPort_PORT), \
    "r" (*_PinPort_DDR | _BitMask), \
    "r" (*_PinPort_PORT & _BitMaskNot) \
  );
}
uint8_t NEO_PIXEL_DRIVER::AddPixel(uint8_t AddSkip) {
  uint8_t PixelAdd = _PixelCount + AddSkip;
  if (!_Init)
    _PixelCount++;
    _PixelCount += AddSkip;
  return PixelAdd;
}
#undef BIT_WRITE_LOW
#undef BIT_WRITE_HIGH
#endif

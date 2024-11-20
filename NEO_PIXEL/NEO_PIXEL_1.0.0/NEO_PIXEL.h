//NEO_PIXEL REV 1.0.0
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

class NEO_PIXEL {
  public:
    NEO_PIXEL(uint8_t Pin, uint8_t LED_Count);
    inline void Write(uint8_t R, uint8_t G, uint8_t B);

  private:
    //Vars for Pin Port Manipulation:
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    volatile uint8_t _BitMask;
    volatile uint8_t _BitMaskNot;

    //This is for malloc for buffer the LED values:
    bool _Init = false;
};

//.cpp
//#include "NEO_PIXEL.h"
//#include "Arduino.h"

NEO_PIXEL::NEO_PIXEL(uint8_t Pin, uint8_t LED_Count) {
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
  digitalWrite(12, LOW);
  pinMode(12, OUTPUT);
  //  asm volatile (
  //    "cbi %1, %2 \n\t"
  //    "sbi %0, %2 \n\t"
  //    ::
  //    "I" (_PinPort_DDR - 0x20),
  //    "I" (_PinPort_PORT - 0x20),
  //    "I" (_PinBit)
  //  );
}

void NEO_PIXEL::Write(uint8_t R, uint8_t G, uint8_t B) {
  noInterrupts();
  uint8_t Low = (*_PinPort_PORT & _BitMaskNot);
  uint8_t High = (*_PinPort_PORT | _BitMask);

  for (uint8_t X = 0; X < 8; X++) {
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)

    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)

    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)

    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)


    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)

    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)


    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)
    BIT_WRITE_HIGH(_PinPort_PORT, Low, High)

    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)


    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
    BIT_WRITE_LOW(_PinPort_PORT, Low, High)
  }
  interrupts();
}
#undef BIT_WRITE_LOW
#undef BIT_WRITE_HIGH
#endif

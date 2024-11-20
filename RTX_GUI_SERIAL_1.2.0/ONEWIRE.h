//ONEWIRE REV 1.0.0
//.h
#ifndef ONEWIRE_h
#define ONEWIRE_h
#include "Arduino.h"
class ONEWIRE {
  public:
    void SetPin(uint8_t Pin);
    void Write(bool *BitArray, uint8_t Length);
    uint8_t Read(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength);

    void PinSet(bool Dir);
    bool PinRead();
    void PinWrite(bool Dir);
  private:
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    uint8_t _PinMask;
    uint8_t _PinMaskNot;
    bool _Boot = false;
};

class DEBUG {
  public:
    DEBUG(uint8_t Pin);

    void PinSet(bool Dir);
    bool PinRead();
    void PinWrite(bool Dir);
    void PinToggle();
  private:
    volatile uint8_t *_PinPort_PIN;
    volatile uint8_t *_PinPort_DDR;
    volatile uint8_t *_PinPort_PORT;
    uint8_t _PinMask;
    uint8_t _PinMaskNot;
};

class BoolConverterClass {
  public:
    void CompileVal(bool *BitArray, uint8_t &BitMarker, uint8_t Val, uint8_t BitsToCompile);
    void CompileArray(bool *BitArray, uint8_t &BitMarker, uint8_t *Array, uint8_t BytesToCompile);
    void DeCompileVal(bool *BitArray, uint8_t &BitMarker, uint8_t &Val, uint8_t BitsToDeCompile);
    void DeCompileArray(bool *BitArray, uint8_t &BitMarker, uint8_t *Array, uint8_t BytesToDeCompile);
};
extern BoolConverterClass BoolConverter;

//.cpp
//#include "ONEWIRE.h"
//#include "Arduino.h"

void ONEWIRE::SetPin(uint8_t Pin) {
  if (Pin <= 19) {
    if (_Boot) {
      *_PinPort_DDR &= _PinMaskNot;
      *_PinPort_PORT &= _PinMaskNot;
    }
    _Boot = true;
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
    *_PinPort_DDR &= _PinMaskNot;
    *_PinPort_PORT |= _PinMask;
    
  }
}

void ONEWIRE::Write(bool *BitArray, uint8_t Length) {
  if (_Boot) {
    const uint8_t StartBitdelay_us = 20;
    const uint8_t SpikeBitDelay_us = 10;
    const uint8_t DataBitDelay_us = 20;

    PinSet(1);
    PinWrite(0);
    delayMicroseconds(StartBitdelay_us);
    PinWrite(1);
    for (uint8_t X = 0; X < Length; X++) {
      delayMicroseconds(SpikeBitDelay_us);
      PinWrite(BitArray[X]);
      delayMicroseconds(DataBitDelay_us);
      PinWrite(!BitArray[X]);
    }
    delayMicroseconds(SpikeBitDelay_us);
    PinWrite(1);
    delayMicroseconds(5);
    PinSet(0);
  }
}

uint8_t ONEWIRE::Read(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength) {
  if (_Boot) {
    uint16_t Counter = 0;
    uint16_t Delay = 20;
    uint16_t DelayMin = 5;
    Delay *= 2.5;
    DelayMin *= 2.5;

    BitCount = 0;
    bool BitHold;

    while (!PinRead() && Counter < Delay) Counter++;
    if (Counter < Delay && Counter > DelayMin) {
      Delay = 15;
      while (BitCount <= MaxLength) {
        delayMicroseconds(18);
        BitHold = PinRead();
        Counter = 0;
        while (PinRead() == BitHold && Counter < Delay) {
          Counter++;
          delayMicroseconds(2);
        }
        if (Counter >= Delay) return 1;
        else {
          if (BitCount < MaxLength)
            BitArray[BitCount] = BitHold;
          BitCount++;
        }
      }
    }
    else if (!(Counter > DelayMin)) return 2;
  }
  return 0;
}

void ONEWIRE::PinSet(bool Dir) {
  if (Dir)
    *_PinPort_DDR |= _PinMask;
  else {
    *_PinPort_DDR &= _PinMaskNot;
    *_PinPort_PORT |= _PinMask;
  }

}

bool ONEWIRE::PinRead() {
  return (*_PinPort_PIN & _PinMask);
}

void ONEWIRE::PinWrite(bool Dir) {
  if (Dir)
    *_PinPort_PORT |= _PinMask;
  else
    *_PinPort_PORT &= _PinMaskNot;
}


DEBUG::DEBUG(uint8_t Pin) {
  if (Pin <= 19) {
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
    *_PinPort_DDR &= _PinMaskNot;
    *_PinPort_PORT &= _PinMaskNot;
  }
}

void DEBUG::PinSet(bool Dir) {
  if (Dir)
    *_PinPort_DDR |= _PinMask;
  else {
    *_PinPort_DDR &= _PinMaskNot;
    *_PinPort_PORT &= _PinMaskNot;
  }
}

bool DEBUG::PinRead() {
  return (*_PinPort_PIN & _PinMask);
}

void DEBUG::PinWrite(bool Dir) {
  if (Dir)
    *_PinPort_PORT |= _PinMask;
  else
    *_PinPort_PORT &= _PinMaskNot;
}

void DEBUG::PinToggle() {
  PinWrite(!PinRead());
}


void BoolConverterClass::CompileVal(bool *BitArray, uint8_t &BitMarker, uint8_t Val, uint8_t BitsToCompile) {
  for (uint8_t X = 0; X < BitsToCompile; X++) {
    BitArray[BitMarker] = (Val & (1 << X));
    BitMarker++;
  }
}
void BoolConverterClass::CompileArray(bool *BitArray, uint8_t &BitMarker, uint8_t *Array, uint8_t BytesToCompile) {
  for (uint8_t X = 0; X < BytesToCompile; X++) {
    BitArray[BitMarker] = (Array[X] & (1 << 0));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 1));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 2));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 3));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 4));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 5));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 6));
    BitMarker++;
    BitArray[BitMarker] = (Array[X] & (1 << 7));
    BitMarker++;
  }
}
void BoolConverterClass::DeCompileVal(bool *BitArray, uint8_t &BitMarker, uint8_t &Val, uint8_t BitsToDeCompile) {
  Val = 0;
  for (uint8_t X = 0; X <  BitsToDeCompile; X++) {
    Val |= (BitArray[BitMarker] << X);
    BitMarker++;
  }
}
void BoolConverterClass::DeCompileArray(bool *BitArray, uint8_t &BitMarker, uint8_t *Array, uint8_t BytesToDeCompile) {
  for (uint8_t X = 0; X < BytesToDeCompile; X++) {
    Array[X] = 0;
    Array[X] |= (BitArray[BitMarker] << 0);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 1);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 2);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 3);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 4);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 5);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 6);
    BitMarker++;
    Array[X] |= (BitArray[BitMarker] << 7);
    BitMarker++;
  }
}
#endif

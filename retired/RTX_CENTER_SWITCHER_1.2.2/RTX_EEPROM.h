//RTX_EEPROM REV 1.0.2
//.h
#ifndef RTX_EEPROM_h
#define RTX_EEPROM_h

//#define EEPROM_WRITE_LED_EN
//#define EEPROM_WRITE_LED_PIN

#include "Arduino.h"
#include "EEPROM.h"

class RTX_EEPROM_Class {

  public:
    bool Read(uint8_t *ByteArray, uint8_t SectorsToRead);
    void Write(uint8_t *ByteArray, uint8_t SectorsToWrite);
    bool WriteRun();
    uint32_t WriteCount();
    uint8_t WriteProg() {
      return _SectorWriteMarker;
    }
    bool Writing() {
      return _Writing;
    }

  private:
    uint8_t *_WriteByteArray;
    uint8_t _SectorsToWrite;
    uint8_t _SectorWriteMarker = 0;
    bool _Writing = false;
    bool _WriteBegin = true;

};
extern RTX_EEPROM_Class RTX_EEPROM;

//.cpp
//#include "RTX_EEPROM.h"
//#include "Arduino.h"

bool RTX_EEPROM_Class::Read(uint8_t *ByteArray, uint8_t SectorsToRead) {
  if (SectorsToRead > 0 && SectorsToRead < 33) {
    bool EEPROM_Correct = true;
    for (uint8_t X = 0; X < SectorsToRead; X++) {
      uint8_t EEPROM_CheckBuffer[8];
      for (uint8_t Y = 0; Y < 8; Y++) {
        EEPROM_CheckBuffer[Y] = EEPROM.read((X * 8) + Y);
        ByteArray[(X * 8) + Y] = EEPROM_CheckBuffer[Y];
      }
      uint8_t V_Parity = EEPROM.read(255 + 1 + (3 * X));
      uint8_t H_Parity = EEPROM.read(255 + 2 + (3 * X));
      uint8_t CheckSum = EEPROM.read(255 + 3 + (3 * X));

      uint8_t COMP_V_Parity = 0;
      uint8_t COMP_H_Parity = 0;
      uint8_t COMP_CheckSum = 0;

      //V Parity
      for (uint8_t A = 0; A < 8; A++) {
        bool ParityCheck = false;
        for (uint8_t B = 0; B < 8; B++)
          if (bitRead(EEPROM_CheckBuffer[A], B)) ParityCheck = !ParityCheck;
        COMP_V_Parity |= (ParityCheck << A);
      }
      //H Parity
      for (uint8_t A = 0; A < 8; A++) {
        bool ParityCheck = false;
        for (uint8_t B = 0; B < 8; B++)
          if (bitRead(EEPROM_CheckBuffer[B], A)) ParityCheck = !ParityCheck;
        COMP_H_Parity |= (ParityCheck << A);
      }
      //Check Sum
      uint16_t TotalCheckSum = 85;
      for (uint8_t A = 0; A < 8; A++) TotalCheckSum += EEPROM_CheckBuffer[A];
      TotalCheckSum += COMP_V_Parity;
      TotalCheckSum += COMP_H_Parity;
      COMP_CheckSum = (TotalCheckSum + (TotalCheckSum >> 8));

      if (COMP_V_Parity != V_Parity || COMP_H_Parity != H_Parity || COMP_CheckSum != CheckSum) EEPROM_Correct = false;
    }
    return EEPROM_Correct;
  }
  return true;
}

void RTX_EEPROM_Class::Write(uint8_t *ByteArray, uint8_t SectorsToWrite) {
  if (SectorsToWrite > 0 && SectorsToWrite < 33) {
    _Writing = true;
    _WriteByteArray = ByteArray;
    _SectorsToWrite = SectorsToWrite;
  }
}

bool RTX_EEPROM_Class::WriteRun() {
  if (_Writing) {
    if (_WriteBegin) {
      uint32_t TimesWritten = 0;
      for (uint8_t X = 0; X < 4; X++) TimesWritten += (EEPROM.read(1023 - X) << (8 * X));
      TimesWritten++;

#ifdef EEPROM_WRITE_LED_EN
      pinMode(EEPROM_WRITE_LED_PIN, OUTPUT);
      digitalWrite(EEPROM_WRITE_LED_PIN, HIGH);
#endif

      for (uint8_t X = 0; X < 4; X++) {
        EEPROM.write(1023 - X, TimesWritten >> (8 * X));
      }

#ifdef EEPROM_WRITE_LED_EN
      digitalWrite(EEPROM_WRITE_LED_PIN, LOW);
      pinMode(EEPROM_WRITE_LED_PIN, INPUT);
#endif

      _WriteBegin = false;
    }
    else {
      if (_SectorWriteMarker < 32) {
        if (_SectorWriteMarker < _SectorsToWrite) {
          //Write Data

#ifdef EEPROM_WRITE_LED_EN
          pinMode(EEPROM_WRITE_LED_PIN, OUTPUT);
          digitalWrite(EEPROM_WRITE_LED_PIN, HIGH);
#endif
          for (uint8_t X = 0; X < 8; X++) EEPROM.write((8 * _SectorWriteMarker) + X, _WriteByteArray[(8 * _SectorWriteMarker) + X]);

#ifdef EEPROM_WRITE_LED_EN
          digitalWrite(EEPROM_WRITE_LED_PIN, LOW);
          pinMode(EEPROM_WRITE_LED_PIN, INPUT);
#endif

          //Error Detection:
          uint8_t V_Parity = 0;
          uint8_t H_Parity = 0;
          uint8_t CheckSum = 0;

          //V Parity
          for (uint8_t A = 0; A < 8; A++) {
            bool ParityCheck = false;
            for (uint8_t B = 0; B < 8; B++)
              if (bitRead(_WriteByteArray[(8 * _SectorWriteMarker) + A], B)) ParityCheck = !ParityCheck;
            V_Parity |= (ParityCheck << A);
          }

          //H Parity
          for (uint8_t A = 0; A < 8; A++) {
            bool ParityCheck = false;
            for (uint8_t B = 0; B < 8; B++)
              if (bitRead(_WriteByteArray[(8 * _SectorWriteMarker) + B], A)) ParityCheck = !ParityCheck;
            H_Parity |= (ParityCheck << A);
          }

          //Check Sum
          uint16_t TotalCheckSum = 85;
          for (uint8_t X = 0; X < 8; X++) TotalCheckSum += _WriteByteArray[(8 * _SectorWriteMarker) + X];
          TotalCheckSum += V_Parity;
          TotalCheckSum += H_Parity;
          CheckSum = (TotalCheckSum + (TotalCheckSum >> 8));

          //Write all Error Detection

#ifdef EEPROM_WRITE_LED_EN
          pinMode(EEPROM_WRITE_LED_PIN, OUTPUT);
          digitalWrite(EEPROM_WRITE_LED_PIN, HIGH);
#endif

          EEPROM.write(255 + 1 + (3 * _SectorWriteMarker), V_Parity);
          EEPROM.write(255 + 2 + (3 * _SectorWriteMarker), H_Parity);
          EEPROM.write(255 + 3 + (3 * _SectorWriteMarker), CheckSum);

#ifdef EEPROM_WRITE_LED_EN
          digitalWrite(EEPROM_WRITE_LED_PIN, LOW);
          pinMode(EEPROM_WRITE_LED_PIN, INPUT);
#endif

        }
        else {

#ifdef EEPROM_WRITE_LED_EN
          pinMode(EEPROM_WRITE_LED_PIN, OUTPUT);
          digitalWrite(EEPROM_WRITE_LED_PIN, HIGH);
#endif

          for (uint8_t X = 0; X < 8; X++) EEPROM.write((8 * _SectorWriteMarker) + X, 0);

          EEPROM.write(255 + 1 + (3 * _SectorWriteMarker), 0);
          EEPROM.write(255 + 2 + (3 * _SectorWriteMarker), 0);
          EEPROM.write(255 + 3 + (3 * _SectorWriteMarker), 85);

#ifdef EEPROM_WRITE_LED_EN
          digitalWrite(EEPROM_WRITE_LED_PIN, LOW);
          pinMode(EEPROM_WRITE_LED_PIN, INPUT);
#endif

        }
        _SectorWriteMarker++;
      }
      else {
        _Writing = false;
        _WriteBegin = true;
        _SectorWriteMarker = 0;
        return false;
      }
    }
    return true;
  }
  else {
    return false;
  }
}

uint32_t RTX_EEPROM_Class::WriteCount() {
  uint32_t TimesWritten = 0;
  for (uint8_t X = 0; X < 4; X++) TimesWritten += (EEPROM.read(1023 - X) << (8 * X));
  return TimesWritten;
}
RTX_EEPROM_Class RTX_EEPROM;
#endif

//DEVICE_BUFFER REV 1.0.1
//.h
#ifndef DEVICE_BUFFER_h
#define DEVICE_BUFFER_h
#include "Arduino.h"

#ifndef DEVICE_BUFFER_DEV_COUNT
#error "YOU HAVE TO DECLARE DEVICE_BUFFER_DEV_COUNT TO PROCEED"
#endif

#ifndef DEVICE_BUFFER_MAX_PCKT
#error "YOU HAVE TO DECLARE DEVICE_BUFFER_MAX_PCKT TO PROCEED"
#endif

#ifndef DEVICE_BUFFER_MAX_BYTE
#error "YOU HAVE TO DECLARE DEVICE_BUFFER_MAX_BYTE TO PROCEED"
#endif

class DEVICE_BUFFER {
  public:

    DEVICE_BUFFER();

    bool ReadAvailable(uint8_t R_TAG);
    uint8_t ReadAvailableBytes(uint8_t R_TAG);
    bool WriteAvailable(uint8_t R_TAG, uint8_t T_TAG);
    uint8_t WriteAvailableBytes(uint8_t R_TAG, uint8_t T_TAG);
    bool Read(uint8_t R_TAG, uint8_t &T_TAG, uint8_t &ByteAvail, uint8_t *Data);
    bool Write(uint8_t R_TAG, uint8_t T_TAG, uint8_t ByteAvail, uint8_t *Data);
    void DeviceFlush(uint8_t TAG);

  private:

    //Check if IP within Range:
    bool _CheckTag(uint8_t TAG);

    //_DataBuffer: [DEV][PCK NO.][PCK CONT.(0-(MX_BYT-1))::BYT CNT(MX_BYT+0)::TX IP(MX_BYT+1)]
    //_ReadMarker: [DEV][BYT_AVL(0)::BYT_MRK(1)]
    volatile uint8_t _DataBuffer[DEVICE_BUFFER_DEV_COUNT][DEVICE_BUFFER_DEV_COUNT * DEVICE_BUFFER_MAX_PCKT][DEVICE_BUFFER_MAX_BYTE + 2];
    volatile uint8_t _ReadMarker[DEVICE_BUFFER_DEV_COUNT][2];


};

//.cpp
//#include "DeviceBuffer.h"
//#include "Arduino.h"

DEVICE_BUFFER::DEVICE_BUFFER() {
  for(uint8_t X = 0; X < DEVICE_BUFFER_DEV_COUNT; X++) this->DeviceFlush(X);
}

bool DEVICE_BUFFER::ReadAvailable(uint8_t R_TAG) {
  if (this->_CheckTag(R_TAG)) {
    if (this->_ReadMarker[R_TAG][0] > 0) return true;
  }
  return false;
}
uint8_t DEVICE_BUFFER::ReadAvailableBytes(uint8_t R_TAG) {
  if (this->_CheckTag(R_TAG)) {
    return this->_ReadMarker[R_TAG][0];
  }
  return 0;
}
bool DEVICE_BUFFER::WriteAvailable(uint8_t R_TAG, uint8_t T_TAG) {
  if (this->_CheckTag(R_TAG) && this->_CheckTag(T_TAG)) {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_ReadMarker[R_TAG][0]; X++) {
      if (this->_DataBuffer[R_TAG][(this->_ReadMarker[R_TAG][1] + X) % (DEVICE_BUFFER_MAX_PCKT * DEVICE_BUFFER_DEV_COUNT)][DEVICE_BUFFER_MAX_BYTE + 1] == T_TAG) WriteCount++;
    }
    if (WriteCount >= DEVICE_BUFFER_MAX_PCKT) return false;
    else return true;
  }
  return false;
}
uint8_t DEVICE_BUFFER::WriteAvailableBytes(uint8_t R_TAG, uint8_t T_TAG) {
  if (this->_CheckTag(R_TAG) && this->_CheckTag(T_TAG)) {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_ReadMarker[R_TAG][0]; X++) {
      if (this->_DataBuffer[R_TAG][(this->_ReadMarker[R_TAG][1] + X) % (DEVICE_BUFFER_MAX_PCKT * DEVICE_BUFFER_DEV_COUNT)][DEVICE_BUFFER_MAX_BYTE + 1] == T_TAG) WriteCount++;
    }
    return (DEVICE_BUFFER_MAX_PCKT - WriteCount);
  }
  return 0;
}
bool DEVICE_BUFFER::Read(uint8_t R_TAG, uint8_t &T_TAG, uint8_t &ByteAvail, uint8_t *Data) {
  if (this->_CheckTag(R_TAG)) {
    if (this->_ReadMarker[R_TAG][0]) {
      for (uint8_t X = 0; X < this->_DataBuffer[R_TAG][this->_ReadMarker[R_TAG][1]][DEVICE_BUFFER_MAX_BYTE + 0]; X++)
        Data[X] = this->_DataBuffer[R_TAG][this->_ReadMarker[R_TAG][1]][X];
      T_TAG = this->_DataBuffer[R_TAG][this->_ReadMarker[R_TAG][1]][DEVICE_BUFFER_MAX_BYTE + 1];
      ByteAvail = this->_DataBuffer[R_TAG][this->_ReadMarker[R_TAG][1]][DEVICE_BUFFER_MAX_BYTE + 0];
      if (this->_ReadMarker[R_TAG][0])  this->_ReadMarker[R_TAG][0]--;
      this->_ReadMarker[R_TAG][1]++;
      if (this->_ReadMarker[R_TAG][1] >= (DEVICE_BUFFER_MAX_PCKT * DEVICE_BUFFER_DEV_COUNT)) this->_ReadMarker[R_TAG][1] = 0;
      return true;
    }
  }
  return false;
}
bool DEVICE_BUFFER::Write(uint8_t R_TAG, uint8_t T_TAG, uint8_t ByteAvail, uint8_t *Data) {
  if (this->_CheckTag(R_TAG) && this->_CheckTag(T_TAG) && ByteAvail <= DEVICE_BUFFER_MAX_BYTE) {
    uint8_t WriteCount = 0;
    for (uint8_t X = 0; X < this->_ReadMarker[R_TAG][0]; X++) {
      if (this->_DataBuffer[R_TAG][(this->_ReadMarker[R_TAG][1] + X) % (DEVICE_BUFFER_MAX_PCKT * DEVICE_BUFFER_DEV_COUNT)][DEVICE_BUFFER_MAX_BYTE + 1] == T_TAG) WriteCount++;
    }
    if(WriteCount < DEVICE_BUFFER_MAX_PCKT) {
      uint8_t WriteMarker = (this->_ReadMarker[R_TAG][1] + this->_ReadMarker[R_TAG][0]) % (DEVICE_BUFFER_MAX_PCKT * DEVICE_BUFFER_DEV_COUNT);
      for(uint8_t X = 0; X < ByteAvail; X++)
        this->_DataBuffer[R_TAG][WriteMarker][X] = Data[X];
      this->_DataBuffer[R_TAG][WriteMarker][DEVICE_BUFFER_MAX_BYTE + 0] = ByteAvail;
      this->_DataBuffer[R_TAG][WriteMarker][DEVICE_BUFFER_MAX_BYTE + 1] = T_TAG;
      this->_ReadMarker[R_TAG][0]++;
      return true;
    }
  }
  return false;
}
void DEVICE_BUFFER::DeviceFlush(uint8_t TAG) {
  if(this->_CheckTag(TAG)) {
    this->_ReadMarker[TAG][0] = 0;
    this->_ReadMarker[TAG][1] = 0;
  }
}

bool DEVICE_BUFFER::_CheckTag(uint8_t TAG) {
  if (TAG < DEVICE_BUFFER_DEV_COUNT) return true;
  else return false;
}

#undef DEVICE_BUFFER_DEV_COUNT
#undef DEVICE_BUFFER_MAX_PCKT
#undef DEVICE_BUFFER_MAX_BYTE
#endif

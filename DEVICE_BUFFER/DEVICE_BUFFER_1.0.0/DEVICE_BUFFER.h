//DEVICE_BUFFER REV 1.0.0
//.h
#ifndef DEVICE_BUFFER_h
#define DEVICE_BUFFER_h
#include "Arduino.h"

class DeviceBuffer {
  public:
    bool Init(uint8_t DevCount, uint8_t MaxPacket, uint8_t MaxByte);
    bool ReadAvailable(uint8_t R_TAG);
    bool ReadAvailable(uint8_t R_TAG, uint8_t &PacketAvail);
    bool WriteAvailable(uint8_t R_TAG, uint8_t T_TAG);
    bool WriteAvailable(uint8_t R_TAG, uint8_t T_TAG, uint8_t &PacketAvail);
    bool Read(uint8_t R_TAG, uint8_t &T_TAG, uint8_t &ByteAvail, uint8_t *Data);
    bool Write(uint8_t R_TAG, uint8_t T_TAG, uint8_t ByteAvail, uint8_t *Data);
    bool DeviceFlush(uint8_t TAG);
    uint16_t BufferSize();

    //Temp:
    //    void SerialBuffer();
    //    void SerialReadAvail();
    //    void SerialWrite();

  private:
    uint16_t _BufferAdd(uint8_t _Dev, uint8_t _Packet, uint8_t _Byte);
    uint16_t _ReadAvailAdd(uint8_t _Dev, bool _Type);
    uint16_t _WriteAdd(uint8_t _Dev1, uint8_t _Dev2);
    bool _CheckTAG(uint8_t _T_TAG1 = 0, uint8_t _T_TAG2 = 0);
    bool _CheckPacket(uint8_t _Packet);
    bool _CheckByte(uint8_t _Byte);
    bool _CheckAlloc();

    bool _BufferAlloc = false;
    uint8_t _DevCount;
    uint8_t _MaxPacket;
    uint8_t _MaxByte;
    uint8_t *_Buffer;
    uint8_t *_ReadAvailMarker;
    uint8_t *_WriteMarker;
};


//.cpp
//#include "DeviceBuffer.h"
//#include "Arduino.h"

bool DeviceBuffer::Init(uint8_t DevCount, uint8_t MaxPacket, uint8_t MaxByte) {
  if (!_BufferAlloc) {
    _Buffer = (uint8_t*) malloc(DevCount * DevCount * MaxPacket * (MaxByte + 2));
    _ReadAvailMarker = (uint8_t*) malloc(DevCount * 2);
    _WriteMarker = (uint8_t*) malloc(DevCount * DevCount);

    if (_Buffer && _ReadAvailMarker && _WriteMarker) {
      _BufferAlloc = true;
      _DevCount = DevCount;
      _MaxPacket = MaxPacket;
      _MaxByte = MaxByte;
      for (uint8_t X = 0; X < _DevCount; X++) DeviceFlush(X);
      return true;
    }
    else {
      _BufferAlloc = false;
      _DevCount = 0;
      _MaxPacket = 0;
      _MaxByte = 0;
    }
  }
  return false;
}

bool DeviceBuffer::ReadAvailable(uint8_t R_TAG) {
  if (_CheckTAG(R_TAG)) {
    return _ReadAvailMarker[_ReadAvailAdd(R_TAG, 1)];
  }
  return false;
}

bool DeviceBuffer::ReadAvailable(uint8_t R_TAG, uint8_t &PacketAvail) {
  if (_CheckTAG(R_TAG)) {
    PacketAvail = _ReadAvailMarker[_ReadAvailAdd(R_TAG, 1)];
    return true;
  }
  PacketAvail = 0;
  return false;
}
bool DeviceBuffer::WriteAvailable(uint8_t R_TAG, uint8_t T_TAG) {
  if (_CheckTAG(R_TAG, T_TAG)) {
    if (_WriteMarker[_WriteAdd(R_TAG, T_TAG)] < _MaxPacket) return true;
  }
  return false;
}
bool DeviceBuffer::WriteAvailable(uint8_t R_TAG, uint8_t T_TAG, uint8_t &PacketAvail) {
  if (_CheckTAG(R_TAG, T_TAG)) {
    if (_WriteMarker[_WriteAdd(R_TAG, T_TAG)] < _MaxPacket) PacketAvail = _MaxPacket - _WriteAdd(R_TAG, T_TAG);
    else PacketAvail = 0;
    return true;
  }
  PacketAvail = 0;
  return false;
}
bool DeviceBuffer::Read(uint8_t R_TAG, uint8_t &T_TAG, uint8_t &ByteAvail, uint8_t *Data) {
  if (ReadAvailable(R_TAG)) {
    T_TAG = _Buffer[_BufferAdd(R_TAG, _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)], _MaxByte + 1)];
    _WriteMarker[_WriteAdd(R_TAG, T_TAG)]--;
    ByteAvail = _Buffer[_BufferAdd(R_TAG, _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)], _MaxByte)];
    for (uint8_t X = 0; X < _MaxByte; X++) Data[X] = _Buffer[_BufferAdd(R_TAG, _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)], X)];
    _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)] = (_ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)] >= (_DevCount * _MaxPacket) - 1) ? 0 : _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)] + 1;
    _ReadAvailMarker[_ReadAvailAdd(R_TAG, 1)]--;
    return true;
  }
  T_TAG = 0;
  ByteAvail = 0;
  return false;
}
bool DeviceBuffer::Write(uint8_t R_TAG, uint8_t T_TAG, uint8_t ByteAvail, uint8_t *Data) {
  if (WriteAvailable(R_TAG, T_TAG)) {
    uint8_t WriteSpot = _ReadAvailMarker[_ReadAvailAdd(R_TAG, 0)] + _ReadAvailMarker[_ReadAvailAdd(R_TAG, 1)];
    WriteSpot -= (WriteSpot >= _DevCount  * _MaxPacket) ? _MaxPacket * _MaxByte : 0;
    for (uint8_t X = 0; X < _MaxByte; X++)
      _Buffer[_BufferAdd(R_TAG, WriteSpot, X)] = Data[X];
    _Buffer[_BufferAdd(R_TAG, WriteSpot, _MaxByte)] = ByteAvail;
    _Buffer[_BufferAdd(R_TAG, WriteSpot, _MaxByte + 1)] = T_TAG;
    _ReadAvailMarker[_ReadAvailAdd(R_TAG, 1)]++;
    _WriteMarker[_WriteAdd(R_TAG, T_TAG)]++;
    return true;
  }
  return false;
}
bool DeviceBuffer::DeviceFlush(uint8_t TAG) {
  if (_CheckTAG(TAG)) {
    _ReadAvailMarker[_ReadAvailAdd(TAG, 0)] = 0;
    _ReadAvailMarker[_ReadAvailAdd(TAG, 1)] = 0;
    for (uint8_t X = 0; X < _DevCount; X++) _WriteMarker[_WriteAdd(TAG, X)] = 0;
  }
  return false;
}



bool DeviceBuffer::_CheckTAG(uint8_t _T_TAG1 = 0, uint8_t _T_TAG2 = 0) {
  if (_BufferAlloc) if (_T_TAG1 < _DevCount && _T_TAG2 < _DevCount) return true;
  return false;
}
bool DeviceBuffer::_CheckPacket(uint8_t _Packet) {
  if (_BufferAlloc) if (_Packet < _DevCount * _MaxPacket) return true;
  return false;
}
bool DeviceBuffer::_CheckByte(uint8_t _Byte) {
  if (_BufferAlloc) if (_Byte < _MaxByte + 2) return true;
  return false;
}
bool DeviceBuffer::_CheckAlloc() {
  if (_BufferAlloc) return true;
  return false;
}




//TEMP:
//void DeviceBuffer::SerialBuffer() {
//  if (_CheckAlloc()) {
//    for (uint8_t X = 0; X < _DevCount; X++) {
//      Serial.print("Device ");
//      Serial.print(X);
//      Serial.println(":");
//      for (uint8_t Y = 0; Y < _MaxPacket * _DevCount; Y++) {
//        Serial.print("[");
//        for (uint8_t Z = 0; Z < _MaxByte + 2; Z++) {
//          Serial.print(_Buffer[_BufferAdd(X, Y, Z)]);
//          if (Z < (_MaxByte + 2) - 1) Serial.print(",");
//        }
//        Serial.print("]");
//        Serial.println();
//      }
//    }
//  }
//}
//void DeviceBuffer::SerialReadAvail() {
//  if (_CheckAlloc()) {
//    for (uint8_t X = 0; X < _DevCount; X++) {
//      Serial.print("Device ");
//      Serial.print(X);
//      Serial.println(":");
//      Serial.print("[");
//      Serial.print(_ReadAvailMarker[_ReadAvailAdd(X, 0)]);
//      Serial.print(",");
//      Serial.print(_ReadAvailMarker[_ReadAvailAdd(X, 1)]);
//      Serial.print("]");
//      Serial.println();
//    }
//  }
//}
//void DeviceBuffer::SerialWrite() {
//  if (_CheckAlloc()) {
//    for (uint8_t X = 0; X < _DevCount; X++) {
//      Serial.print("Device ");
//      Serial.print(X);
//      Serial.println(":");
//      Serial.print("[");
//      for (uint8_t Y = 0; Y < _DevCount; Y++) {
//        Serial.print(_WriteMarker[_WriteAdd(X, Y)]);
//        if (Y < _DevCount - 1) Serial.print(",");
//      }
//      Serial.print("]");
//      Serial.println();
//    }
//  }
//}
//:TEMP





uint16_t DeviceBuffer::_BufferAdd(uint8_t _Dev, uint8_t _Packet, uint8_t _Byte) {
  //  if (_CheckTAG(_Dev) && _CheckPacket(_Packet) && _CheckByte(_Byte))
  return (_Dev * (_DevCount * _MaxPacket) * (_MaxByte + 2)) + (_Packet * (_MaxByte + 2)) + _Byte;
  //  return 0;
}

uint16_t DeviceBuffer::_ReadAvailAdd(uint8_t _Dev, bool _Type) {
  //  if (_CheckTAG(_Dev))
  return (_Dev * 2) + _Type;
  //  return 0;
}

uint16_t DeviceBuffer::_WriteAdd(uint8_t _Dev1, uint8_t _Dev2) {
  //  if (_CheckTAG(_Dev1, _Dev2))
  return (_Dev1 * _DevCount) + _Dev2;
  //  return 0;
}

uint16_t DeviceBuffer::BufferSize() {
  if (_BufferAlloc) return (_DevCount * _DevCount * _MaxPacket * (_MaxByte + 2)) + (_DevCount * 2);
  else return 0;
}
#endif

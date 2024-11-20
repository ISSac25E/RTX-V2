//ONEWIRE_MASTER REV 1.2.2_1
//.h
#ifndef ONEWIRE_MASTER_h
#define ONEWIRE_MASTER_h

#ifndef ONEWIRE_DATA_PIN
#error "YOU HAVE TO DECLARE ONEWIRE_DATA_PIN TO PROCEED"
#endif

#ifndef ONEWIRE_SLAVE_DEV_CNT
#error "YOU HAVE TO DECLARE ONEWIRE_SLAVE_DEV_CNT TO PROCEED"
#endif

#ifndef ONEWIRE_SLAVE_DEV_IP
#error "YOU HAVE TO DECLARE ONEWIRE_SLAVE_DEV_IP TO PROCEED"
#endif

#ifndef DEVICE_BUFFER_DEV_COUNT
#define DEVICE_BUFFER_DEV_COUNT ONEWIRE_SLAVE_DEV_CNT //Default
#endif

#ifndef DEVICE_BUFFER_MAX_PCKT
#define DEVICE_BUFFER_MAX_PCKT 1 //Default
#endif

#ifndef DEVICE_BUFFER_MAX_BYTE
#define DEVICE_BUFFER_MAX_BYTE 10 //Default
#endif

#include "Arduino.h"
#include "DEVICE_BUFFER.h"
#include "ONEWIRE_DRIVER.h"

/*
   RunStatus:
    0 = NO MSG
    1 = CONNECT FST MSG
    2 = ACK CONNECT MSG
    3 = Read !Avail
    4 = Read Success
    5 = Write !Connected
    6 = BUFF FULL
    7 = Write Success
    8 = MX (N/A)
*/

class ONEWIRE_MASTER {
  public:

    ONEWIRE_MASTER() {
      _PIN.SetPin(ONEWIRE_DATA_PIN);
      for (uint8_t X = 0; X < ONEWIRE_SLAVE_DEV_CNT; X++) _ConnectedDevice[X] = false;
    }

    void Run(uint8_t *RunStatus);
    bool DeviceConnected(uint8_t Device) {
      return _ConnectedDevice[Device % _SlaveCount];
    };
  private:
    DEVICE_BUFFER _SLAVE_BUFFER;
    ONEWIRE _PIN;

    bool _OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime);
    bool _CheckConnected(uint8_t SlaveIP, uint8_t COM);
    bool _CheckIP(uint8_t IP, uint8_t &IP_TAG);
    bool _OneWireSafe();

    const uint8_t _SlaveIP[ONEWIRE_SLAVE_DEV_CNT] = ONEWIRE_SLAVE_DEV_IP;
    const uint8_t _SlaveCount = ONEWIRE_SLAVE_DEV_CNT;
    bool _ConnectedDevice[ONEWIRE_SLAVE_DEV_CNT];
    uint32_t _ConnectedTimer[ONEWIRE_SLAVE_DEV_CNT];
};

//.cpp
//#include "ONEWIRE_MASTER.h"
//#include "Arduino.h"

void ONEWIRE_MASTER::Run(uint8_t *RunStatus) {
  for (uint8_t X = 0; X < _SlaveCount; X++) {
    RunStatus[X] = 0;
    if (_CheckConnected(X, 2)) {
      bool BitArray[88];
      uint8_t BitMarker = 0;
      BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
      {
        //uint8_t Command = 1;
        //Command |= (_SlaveBuffer.ReadAvailable(X) << 1);
        //BoolConverter.CompileVal(BitArray, BitMarker, Command, 2);

        BoolConverter.CompileVal(BitArray, BitMarker, 1, 1);
        BoolConverter.CompileVal(BitArray, BitMarker, _SLAVE_BUFFER.ReadAvailable(X), 1);
      }
      if (_OneWireSafe()) {
        _PIN.Write(BitArray, BitMarker);
        //check if safe
        //Send message
        if (_OneWireParseRead(BitArray, BitMarker, 88, 40)) {
          if (BitMarker == 5) {
            uint8_t SlaveIP;
            uint8_t Command;
            uint8_t DeCompileMarker = 0;
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, SlaveIP, 3);
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, Command, 2);
            if (SlaveIP == _SlaveIP[X]) {
              if (Command == 0) {
                RunStatus[X] = 2;
                _CheckConnected(X, 1);
                BitMarker = 0;
                BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                {
                  //uint8_t Command = 1;
                  //Command |= (_SlaveBuffer.ReadAvailable(X) << 1);
                  //BoolConverter.CompileVal(BitArray, BitMarker, Command, 2);

                  BoolConverter.CompileVal(BitArray, BitMarker, 1, 1);
                  BoolConverter.CompileVal(BitArray, BitMarker, _SLAVE_BUFFER.ReadAvailable(X), 1);
                }
                //Message
                _PIN.Write(BitArray, BitMarker);
                //Buffer a little
                delayMicroseconds(60);
              }
              else if (Command == 1) {
                _CheckConnected(X, 1);
                uint8_t SenderIP;
                uint8_t BytesAvail;
                uint8_t DataArray[10];
                bool ReadAvail = _SLAVE_BUFFER.Read(X, SenderIP, BytesAvail, DataArray);
                BitMarker = 0;
                BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                BoolConverter.CompileVal(BitArray, BitMarker, ReadAvail, 2);
                if (ReadAvail) {
                  RunStatus[X] = 4;
                  BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[SenderIP], 3);
                  BoolConverter.CompileArray(BitArray, BitMarker, DataArray, BytesAvail);
                }
                else RunStatus[X] = 3;
                //Send Message
                _PIN.Write(BitArray, BitMarker);
                //Buffer a little
                delayMicroseconds(60);
              }
            }
            //Read or Connect Command
          }
          else if (BitMarker % 8 == 0 && BitMarker >= 8) {
            uint8_t SlaveIP;
            uint8_t Command;
            uint8_t DeCompileMarker = 0;
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, SlaveIP, 3);
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, Command, 2);
            if (SlaveIP == _SlaveIP[X]) {
              if (Command == 2) {
                uint8_t RecieverSlaveIP;
                uint8_t RecieverIP_TAG;
                BoolConverter.DeCompileVal(BitArray, DeCompileMarker, RecieverSlaveIP, 3);
                if (_CheckIP(RecieverSlaveIP, RecieverIP_TAG)) {
                  _CheckConnected(X, 1);
                  uint8_t DataArray[((BitMarker - 8) / 8)];
                  BoolConverter.DeCompileArray(BitArray, DeCompileMarker, DataArray, ((BitMarker - 8) / 8));
                  if (_CheckConnected(RecieverIP_TAG, 2)) {
                    bool WriteStatus = _SLAVE_BUFFER.Write(RecieverIP_TAG, X, ((DeCompileMarker - 8) / 8), DataArray);
                    DeCompileMarker = BitMarker;
                    BitMarker = 0;
                    BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                    BoolConverter.CompileVal(BitArray, BitMarker, WriteStatus, 1);
                    BoolConverter.CompileVal(BitArray, BitMarker, 0, 1);
                    //Send Message
                    if(WriteStatus) RunStatus[X] = 7;
                    else RunStatus[X] = 7;
                    _PIN.Write(BitArray, BitMarker);
                  }
                  else {
                    RunStatus[X] = 5;
                    BitMarker = 0;
                    BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                    BoolConverter.CompileVal(BitArray, BitMarker, 2, 2);
                    //Send Message
                    _PIN.Write(BitArray, BitMarker);
                  }
                  //Buffer a little
                  delayMicroseconds(60);
                  //Write Command
                }
              }
            }
            //Write Command
          }
          else if (BitMarker % 8 == 5 && BitMarker >= 5) {
            //MX Command?
            RunStatus[X] = 8;
            //TODO: Write MX Protocol
          }
        }
      }
    }
    else {
      //Not Connected
      bool BitArray[5];
      uint8_t BitMarker = 0;
      BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
      BoolConverter.CompileVal(BitArray, BitMarker, 0, 2);
      if (_OneWireSafe()) {
        _PIN.Write(BitArray, BitMarker);
        //        check if safe
        //        Send message
        if (_OneWireParseRead(BitArray, BitMarker, 5, 40)) {
          if (BitMarker == 5) {
            uint8_t SlaveIP;
            uint8_t Command;
            uint8_t DeCompileMarker = 0;
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, SlaveIP, 3);
            BoolConverter.DeCompileVal(BitArray, DeCompileMarker, Command, 2);
            if (SlaveIP == _SlaveIP[X]) {
              if (Command == 0) {
                RunStatus[X] = 1;
                _CheckConnected(X, 1);
                BitMarker = 0;
                BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                {
                  //                  uint8_t Command = 1;
                  //                  Command |= (_SlaveBuffer.ReadAvailable(X) << 1);
                  //                  BoolConverter.CompileVal(BitArray, BitMarker, Command, 2);

                  BoolConverter.CompileVal(BitArray, BitMarker, 1, 1);
                  BoolConverter.CompileVal(BitArray, BitMarker, _SLAVE_BUFFER.ReadAvailable(X), 1);
                }
                //Send Message
                _PIN.Write(BitArray, BitMarker);
                //Buffer a little
                delayMicroseconds(60);
              }
            }
            //Connect Command
          }
        }
      }
    }
  }
}

bool ONEWIRE_MASTER::_CheckConnected(uint8_t SlaveIP, uint8_t COM) {
  if (SlaveIP < _SlaveCount) {
    switch (COM) {
      case 0:
        _ConnectedDevice[SlaveIP] = false;
        _ConnectedTimer[SlaveIP] = millis();
        return false;
      case 1:
        _ConnectedDevice[SlaveIP] = true;
        _ConnectedTimer[SlaveIP] = millis();
        return true;
      case 2:
        if (_ConnectedDevice[SlaveIP]) {
          if ((millis() - _ConnectedTimer[SlaveIP]) <= 3000) return true;
          else {
            _ConnectedDevice[SlaveIP] = false;
            _ConnectedTimer[SlaveIP] = millis();
            _SLAVE_BUFFER.DeviceFlush(SlaveIP);
            return false;
          }
        }
        else return false;
    }
  }
  return false;
}

bool ONEWIRE_MASTER::_OneWireSafe() {
  uint16_t Counter = 0;
  uint16_t Delay = 20;
  while (Counter < Delay and _PIN.PinRead()) {
    Counter++;
    delayMicroseconds(2);
  }
  if (Counter >= Delay) return true;
  else return false;
}

bool ONEWIRE_MASTER::_OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime) {
  uint16_t Counter = 0;
  uint16_t Delay = MaxParseTime;
  Delay *= 2.5;
  while (Counter < Delay) {
    Counter++;
    if (!_PIN.PinRead()) {
      uint8_t ReadStatus = _PIN.Read(BitArray, BitCount, MaxLength);
      if (ReadStatus == 0) return false;
      if (ReadStatus == 1) return true;
    }
  }
  return false;
}

bool ONEWIRE_MASTER::_CheckIP(uint8_t IP, uint8_t &IP_TAG) {
  for (uint8_t X = 0; X < _SlaveCount; X++) {
    if (IP == _SlaveIP[X]) {
      IP_TAG = X;
      return true;
    }
  }
  IP_TAG = 0;
  return false;
}

//#undef ONEWIRE_DATA_PIN
//#undef ONEWIRE_SLAVE_DEV_CNT
//#undef ONEWIRE_SLAVE_DEV_IP
#endif

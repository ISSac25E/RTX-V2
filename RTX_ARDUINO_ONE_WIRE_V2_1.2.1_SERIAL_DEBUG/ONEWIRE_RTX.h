//ONEWIRE_RTX REV 1.0.0_1
//.h
#ifndef ONEWIRE_RTX_h
#define ONEWIRE_RTX_h

#include "Arduino.h"
#include "DeviceBuffer.h"
#include "ONEWIRE.h"

#define RTX_MAX_PACKET_HOLD 1

/*
   SLAVEDEVICE:
   |STATUS CODES:
   |  (bool) Status(uint8_t):
   |   {ERROR, Connected, MasterError, DuplicateIP, RX, TX, TX-R}
   |     [0]      [1]         [2]           [3]     [4] [5]  [6]
*/
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

DEBUG _DebugPin(13);

//Global:
DeviceBuffer _SlaveBuffer;

class ONEWIRE_Master {
  public:
//    DeviceBuffer _SlaveBuffer;
    ONEWIRE _PIN;
  public:
    bool SetDevices(uint8_t* SlaveDevices, uint8_t DeviceCount);
    void SetPin(uint8_t Pin);
    void Run(uint8_t *RunStatus);
    bool DeviceConnected(uint8_t Device) {
      return _ConnectedDevice[Device % 8];
    };
  private:
    bool _OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime);
    bool _CheckConnected(uint8_t SlaveIP, uint8_t COM);
    bool _CheckIP(uint8_t IP, uint8_t &IP_TAG);
    bool _TwoWireSafe();

    bool _DeviceBoot = false;
    bool _PinBoot = false;
    volatile uint8_t _SlaveIP[8];
    uint8_t _SlaveCount;
    volatile bool _ConnectedDevice[8] = {false, false, false, false, false, false, false, false};
    volatile uint32_t _ConnectedTimer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
};

class ONEWIRE_Slave {
  private:
    ONEWIRE _PIN;
  public:
    void SetPin(uint8_t Pin);
    void SetIP(uint8_t IP);

    bool Run();
    bool Read(uint8_t &SenderIP, uint8_t &ByteCount, uint8_t *DataArray);
    bool Write(uint8_t RecieverIP, uint8_t ByteCount, uint8_t *DataArray);
    bool Status(uint8_t CMD);

  private:
    bool _OneWireParsePacket(bool *BitArray);
    uint8_t _OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime);
    bool _OneWireInsert();

    uint8_t _DeviceIP;
    bool _PinBoot = false;
    bool _DeviceBoot = false;
    bool _Connected = false;
    bool _Status[5]; //{MasterError, DuplicateIP, RX, TX, TX-R}
    uint32_t ConnectedTimer;
};

//.cpp
//#include "ONEWIRE_RTX.h"
//#include "Arduino.h"

void ONEWIRE_Slave::SetPin(uint8_t Pin) {
  _PIN.SetPin(Pin);
  _PinBoot = true;
  _DebugPin.PinSet(HIGH);
}

void ONEWIRE_Slave::SetIP(uint8_t IP) {
  _DeviceIP = 0;
  _DeviceIP = (IP & B00000111);
  _DeviceBoot = true;
}

bool ONEWIRE_Slave::Status(uint8_t CMD) {
  switch (CMD % 7) {
    case 0:
      //ERROR
      if (!_Connected || _Status[0] || _Status[1]) return true;
      break;
    case 1:
      //Connected
      return _Connected;
      break;
    case 2:
      //Master Error
      return _Status[0];
      break;
    case 3:
      //DuplicateIP
      return _Status[1];
      break;
    case 4:
      //RX
      return _Status[2];
      break;
    case 5:
      //TX
      return _Status[3];
      break;
    case 6:
      //TX_R
      return _Status[4];
      //True == Buffer Full on Reciever
      break;
  }
  return false;
}

bool ONEWIRE_Slave::Run() {
  //Reset Status Values:
  _Status[0] = false; //MasterError
  _Status[1] = false; //DuplicateIP Error
  if (_DeviceBoot && _PinBoot) {
    uint32_t MillisTimer = millis();
    //WatchDog Timer
    if (_Connected && (MillisTimer - ConnectedTimer) >= 2000) {
      //WatchDog Timer Triggered
      _Connected = false;
    }
    if (!_Connected || (MillisTimer - ConnectedTimer) >= 1000) {
      bool BitArrayRead[5];
      bool BitArraySend[5];
      uint8_t BitCountSend = 0;
      BoolConverter.CompileVal(BitArraySend, BitCountSend, _DeviceIP, 3);
      BoolConverter.CompileVal(BitArraySend, BitCountSend, 0, 2);
      if (_OneWireParsePacket(BitArrayRead)) {
        uint8_t BitMarker = 3;
        uint8_t Command;
        BoolConverter.DeCompileVal(BitArrayRead, BitMarker, Command, 1);
        if (!_Connected && Command) {
          //DuplicateIP Error, Break immediately
          _Status[1] = true;
          _Connected = false;
          return false;
        }
        _PIN.Write(BitArraySend, BitCountSend);
        //We are Connected already, Send update Connection packet
        //Check for response:
        if (_OneWireParseRead(BitArrayRead, BitMarker, 5, 300) == 1) {
          if (BitMarker == 5) {
            BitMarker = 0;
            uint8_t IP;
            uint8_t Command;
            BoolConverter.DeCompileVal(BitArrayRead, BitMarker, IP, 3);
            BoolConverter.DeCompileVal(BitArrayRead, BitMarker, Command, 1);
            if (IP == _DeviceIP) {
              if (Command) {
                ConnectedTimer = millis();
                _Connected = true;
                return true;
                //Success, Connected
              }
              else {
                _Connected = false;
                return false;
                //Not Connected
              }
            }
            else {
              //No confirmation, Error?
              return _Connected;
            }
          }
          else {
            //Not Correct Packet Size, Error?
            return _Connected;
          }
        }
        else {
          //No return response/corrupted Packet, Error?
          return _Connected;
        }
      }
      else {
        //Connected = false;
        //couldn't parse packet, Error?
        //        _Status[0] = true;
        return _Connected;
      }
    }
    else {
      return true;
    }
  }
  _Connected = false;
  return false;
}

bool ONEWIRE_Slave::Read(uint8_t &SenderIP, uint8_t &ByteCount, uint8_t *DataArray) {
  if (_DeviceBoot && _PinBoot) {
    //Reset Status Values:
    _Status[2] = false; //RX
    _Status[3] = false; //TX
    _Status[4] = false; //TX-R
    bool MessageAvail = false;
    if (_Connected) {
      bool BitArrayRead[88];
      bool BitArraySend[5];
      uint8_t BitCountSend = 0;
      uint8_t BitCountRead = 3;
      BoolConverter.CompileVal(BitArraySend, BitCountSend, _DeviceIP, 3);
      BoolConverter.CompileVal(BitArraySend, BitCountSend, 1, 2);
      if (_OneWireParsePacket(BitArrayRead)) {
        uint8_t Command;
        BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, Command, 1);
        if (Command) {
          //We are connected
          BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, Command, 1);
          if (Command) {
            //Message available in buffer, send message to retrieve
            _PIN.Write(BitArraySend, BitCountSend);
            if (_OneWireParseRead(BitArrayRead, BitCountRead, 88, 300)) {
              //Message Recieved, check all requirments check out
              if (BitCountRead % 8 == 0 && BitCountRead >= 8) {
                uint8_t BitMarker = 0;
                uint8_t IP;
                BoolConverter.DeCompileVal(BitArrayRead, BitMarker, IP, 3);
                if (IP == _DeviceIP) {
                  BoolConverter.DeCompileVal(BitArrayRead, BitMarker, Command, 2);
                  if (Command == 1) {
                    //Message is available for reading
                    ByteCount = ((BitCountRead - 8) / 8);
                    uint8_t ReturnSenderIP;
                    BoolConverter.DeCompileVal(BitArrayRead, BitMarker, ReturnSenderIP, 3);
                    SenderIP = ReturnSenderIP;
                    BoolConverter.DeCompileArray(BitArrayRead, BitMarker, DataArray, ((BitCountRead - 8) / 8));
                    _Status[2] = true;
                    MessageAvail = true;
                  }
                  //else, Message is not available
                  //Update ConnectedTimer Anyway
                  ConnectedTimer = millis();
                }
                //else, Wrong IP
              }
              else if (BitCountRead == 5) {
                //Message Not available
                //Check to at least update ConnectedTimer
                uint8_t BitMarker = 0;
                uint8_t IP;
                BoolConverter.DeCompileVal(BitArrayRead, BitMarker, IP, 3);
                if (IP == _DeviceIP) {
                  BoolConverter.DeCompileVal(BitArrayRead, BitMarker, Command, 2);
                  if (Command == 0) {
                    //No Message
                    //Update ConnectedTimer Anyway
                  }
                  ConnectedTimer = millis();
                }
                //else, Wrong IP
              }
              //else, Wrong packet size, maybe corrupted?
            }
          }
          else {
            //Message not available, flag error codes
          }
        }
        else {
          _Connected = false;
          //Not Connected, Flag Error Codes
        }
      }
      else {
        _Connected = false;
        //Master Error, counld not insert
      }
    }
    Run();
    return MessageAvail;
  }
  _Connected = false;
  return false;
}

bool ONEWIRE_Slave::Write(uint8_t RecieverIP, uint8_t ByteCount, uint8_t *DataArray) {
  if (_DeviceBoot && _PinBoot) {
    //Reset Status Values:
    _Status[2] = false; //RX
    _Status[3] = false; //TX
    _Status[4] = false; //TX-R
    bool MessageWritten = false;
    if (_Connected) {
      bool BitArrayRead[5];
      bool BitArraySend[(ByteCount * 8) + 8];
      uint8_t BitCountSend = 0;
      uint8_t BitCountRead = 3;
      BoolConverter.CompileVal(BitArraySend, BitCountSend, _DeviceIP, 3);
      BoolConverter.CompileVal(BitArraySend, BitCountSend, 2, 2);
      BoolConverter.CompileVal(BitArraySend, BitCountSend, RecieverIP, 3);
      BoolConverter.CompileArray(BitArraySend, BitCountSend, DataArray, ByteCount);
      if (_OneWireParsePacket(BitArrayRead)) {
        uint8_t Command;
        BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, Command, 1);
        if (Command) {
          //We are connected
          _PIN.Write(BitArraySend, BitCountSend);
          if (_OneWireParseRead(BitArrayRead, BitCountRead, 5, 300)) {
            //Message Recieved, check all requirments check out
            if (BitCountRead == 5) {
              uint8_t IP;
              BitCountRead = 0;
              BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, IP, 3);
              if (IP == _DeviceIP) {
                BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, Command, 1);
                if (Command) {
                  //Write Succsesful
                  _Status[3] = true;
                  MessageWritten = true;
                }
                else {
                  //Write UnSuccsesful
                  //Get reason of failed write
                  //update (Command) into error codes:
                  BoolConverter.DeCompileVal(BitArrayRead, BitCountRead, Command, 1);
                  if (!Command) _Status[4] = true;
                }
                //Update ConnectionTimer:
                ConnectedTimer = millis();
              }
              //else, Wrong IP
            }
            //else, Wrong packet size, maybe corrupted?
          }
        }
        else {
          _Connected = false;
          //Not Connected, Flag Error Codes
        }
      }
      else {
        _Connected = false;
        //Master Error, counld not insert
      }
    }
    Run();
    return MessageWritten;
  }
  _Connected = false;
  return false;
}

bool ONEWIRE_Slave::_OneWireParsePacket(bool *BitArray) {
  uint8_t BitCount;
  const uint32_t Timer = millis();
  const uint32_t Delay = 20;
  while ((millis() - Timer) < Delay) {
    if (_OneWireInsert()) {
      uint8_t ReadStatus = _OneWireParseRead(BitArray, BitCount, 5, 300);
      if (ReadStatus == 1) {
        if (BitCount == 5) {
          BitCount = 0;
          uint8_t IP;
          BoolConverter.DeCompileVal(BitArray, BitCount, IP, 3);
          if (IP == _DeviceIP) return true;
          //else, wrong packet keep looking
        }
      }
      else if (ReadStatus == 2) {
        //Master Error
        _Status[0] = true;
        return false;
      }
      //If ReadStatus == 0, wrong packet size, keep looking
    }
  }
  //Could not find correct packet:
  //Master Error
  return false;
}

uint8_t ONEWIRE_Slave::_OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime) {
  uint16_t Counter = 0;
  uint16_t Delay = MaxParseTime;
  Delay *= 2.5;
  while (Counter < Delay) {
    Counter++;
    if (!_PIN.PinRead()) {
      uint8_t ReadStatus = _PIN.Read(BitArray, BitCount, MaxLength);
      if (ReadStatus == 0) return 0;
      if (ReadStatus == 1) return 1;
    }
  }
  return 2;
}

bool ONEWIRE_Slave::_OneWireInsert() {
  uint16_t Counter = 0;
  uint16_t Delay = 40;
  while (Counter < Delay and _PIN.PinRead()) {
    Counter++;
    delayMicroseconds(2);
  }
  if (Counter >= Delay) return true;
  else return false;
}



void ONEWIRE_Master::SetPin(uint8_t Pin) {
  _DebugPin.PinSet(true);
  _PIN.SetPin(Pin);
  _PinBoot = true;
}

bool ONEWIRE_Master::SetDevices(uint8_t* SlaveIP, uint8_t SlaveCount) {
  if (SlaveCount > 8) _SlaveCount = 8;
  else _SlaveCount = SlaveCount;
  for (uint8_t X = 0; X < _SlaveCount; X++) _SlaveIP[X] = SlaveIP[X];
  _DeviceBoot = _SlaveBuffer.Init(_SlaveCount, RTX_MAX_PACKET_HOLD, 10);
  return _DeviceBoot;
}

void ONEWIRE_Master::Run(uint8_t* RunStatus) {
  if (_PinBoot && _DeviceBoot) {
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
          BoolConverter.CompileVal(BitArray, BitMarker, _SlaveBuffer.ReadAvailable(X), 1);
        }
        if (_TwoWireSafe()) {
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
                    BoolConverter.CompileVal(BitArray, BitMarker, _SlaveBuffer.ReadAvailable(X), 1);
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
                  bool ReadAvail = _SlaveBuffer.Read(X, SenderIP, BytesAvail, DataArray);
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
                      DeCompileMarker = BitMarker;
                      BitMarker = 0;
                      bool WriteStat = _SlaveBuffer.Write(RecieverIP_TAG, X, ((DeCompileMarker - 8) / 8), DataArray);
                      BoolConverter.CompileVal(BitArray, BitMarker, _SlaveIP[X], 3);
                      BoolConverter.CompileVal(BitArray, BitMarker, WriteStat, 1);
                      BoolConverter.CompileVal(BitArray, BitMarker, 0, 1);
                      //Send Message
                      _PIN.Write(BitArray, BitMarker);
                      if (WriteStat) {
                        RunStatus[X] = 7;
                      }
                      else {
                        RunStatus[X] = 6;
                      }
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
              RunStatus[X] = 8;
              //MX Command?
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
        if (_TwoWireSafe()) {
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
                    BoolConverter.CompileVal(BitArray, BitMarker, _SlaveBuffer.ReadAvailable(X), 1);
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
}

bool ONEWIRE_Master::_CheckConnected(uint8_t SlaveIP, uint8_t COM) {
  if (SlaveIP < _SlaveCount) {
    switch (COM) {
      case 0:
        _ConnectedDevice[SlaveIP] = false;
        _ConnectedTimer[SlaveIP] = millis();
        return false;
        break;
      case 1:
        _ConnectedDevice[SlaveIP] = true;
        _ConnectedTimer[SlaveIP] = millis();
        return true;
        break;
      case 2:
        if (_ConnectedDevice[SlaveIP]) {
          if ((millis() - _ConnectedTimer[SlaveIP]) <= 3000) return true;
          else {
            _ConnectedDevice[SlaveIP] = false;
            _SlaveBuffer.DeviceFlush(SlaveIP);
            return false;
          }
        }
        else return false;
        break;
      default:
        break;
    }
  }
  return false;
}

bool ONEWIRE_Master::_TwoWireSafe() {
  uint16_t Counter = 0;
  uint16_t Delay = 20;
  while (Counter < Delay and _PIN.PinRead()) {
    Counter++;
    delayMicroseconds(2);
  }
  if (Counter >= Delay) return true;
  else return false;
}

bool ONEWIRE_Master::_OneWireParseRead(bool *BitArray, uint8_t &BitCount, uint8_t MaxLength, uint16_t MaxParseTime) {
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

bool ONEWIRE_Master::_CheckIP(uint8_t IP, uint8_t &IP_TAG) {
  for (uint8_t X = 0; X < _SlaveCount; X++) {
    if (IP == _SlaveIP[X]) {
      IP_TAG = X;
      return true;
    }
  }
  IP_TAG = 0;
  return false;
}
#undef RTX_MAX_PACKET_HOLD
#endif

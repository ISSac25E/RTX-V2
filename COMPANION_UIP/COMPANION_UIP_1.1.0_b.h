// COMPANION_UIP REV 1.1.0 beta
//.h
/*
  - Change to one port communication
      *Apparently not possible or very difficult to do
      Some kind of issue with the enc28j60 or UIP Library or unknown config that causes issues with sending AND receiving packets over a single UDP port
  - No connection Sleep
  - Simplify lib
  - receive feedback from companion
      each command from companion must be sent as a separate packet
      The first characters/word indicate the packet command:
        The only command that this lib in concerned about would be "ack"
        For RTX, commands such as "cb" and "db" would be used for config buffer and debug buffer respectively
*/
#ifndef COMPANION_UIP_h
#define COMPANION_UIP_h

#ifndef COMPANION_SEND_INTERVAL
#define COMPANION_SEND_INTERVAL 300 // in ms
#endif
#ifndef COMPANION_CONNECTION_TIMEOUT
#define COMPANION_CONNECTION_TIMEOUT 1000 // in ms
#endif
#ifndef COMPANION_AckPacket_BUFFER
#define COMPANION_AckPacket_BUFFER 20 // largest command packet possible
#endif

#include "Arduino.h"

#include <SPI.h>
#include <UIPEthernet.h>
#include <UIPUdp.h>

UIPUDP Comp_udp_rx;
UIPUDP Comp_udp_tx;

class COMP_UIP
{
public:
  // 0 = Disconnected, 1 = Connected:
  bool Run();
  /*
    GetCommand():
      Provide byte buffer and max bytes allowed to read.
      returns number of bytes available to read

      If command packet happens to exceed max allowed bytes, 0 will be returned, no packet will be provided
      This packet will be available until 'run' is called again or ethernet is reset through 'setup' 'connect' or 'disconnect'
  */
  uint8_t GetCommand(byte *, uint8_t);
  /*
    Setup():
    this will need the Arduino listen port. The outgoing port will be the same
    - Companion listen IPAddress
    - Companion listen Port
    - Arduino listen Port
  */
  void Setup(IPAddress Comp_IP, uint16_t Comp_Port, uint16_t Ard_Port)
  {
    _Comp_IP = Comp_IP;
    _Comp_Port = Comp_Port;
    _Ard_Port = Ard_Port;

    this->Connect();
  }
  void Connect()
  {
    _Init = true;
    _ReadPacket.ackPacket = false;
    _Connected = false;
    _Timer = millis();
    _CheckTimer = millis();
    Comp_udp_rx.stop();
    Comp_udp_tx.stop();
    Comp_udp_rx.begin(_Ard_Port);
    Comp_udp_tx.begin(_Ard_Port);
  }
  void Disconnect()
  {
    _Init = false;
    _ReadPacket.ackPacket = false;
    _Connected = false;
    Comp_udp_rx.stop();
    Comp_udp_tx.stop();
  }

  void SetConnectButton(uint8_t Page, uint8_t Bank)
  {
    if (Page > 0 && Page < 100 && Bank > 0 && Bank < 33)
    {
      _ConnectPage = Page;
      _ConnectBank = Bank;
    }
  }

  void PressButton(uint8_t Page, uint8_t Bank, uint8_t Dir);

private:
  IPAddress _Comp_IP = IPAddress(0, 0, 0, 0);
  uint16_t _Comp_Port;
  uint16_t _Ard_Port;
  uint8_t _ConnectPage = 0;
  uint8_t _ConnectBank = 0;
  bool _Init = false;
  bool _Connected = false;
  uint32_t _Timer = 0;
  uint32_t _CheckTimer = 0;

  // this may be over engineered junk, but its a good exercise:
  struct
  {
    bool ackPacket = false;
    bool packetAvailable = false;
    bool packetReadBegin = false;
    byte packetBuffer[3];
  } _ReadPacket;

  /*
    Simplified version of strcmp
      give two byte arrays, and state how many byte are needed to be compared

      Returns: 0 if no compare
      Returns: 1 if successful compare

      Converts all bytes to lowercase
  */
  static bool charComp(byte *c1, byte *c2, uint8_t len)
  {
    bool ret = true;
    for (uint8_t x = 0; x < len; x++)
    {
      if (toLowerCase(c1[x]) != toLowerCase(c2[x]))
      {
        ret = false;
        break;
      }
    }
    return ret;
  }
};

//.cpp
// #include "COMPANION_UIP.h"
// #include "Arduino.h"

bool COMP_UIP::Run()
{
  if (_Init)
  {
    if (Comp_udp_rx.parsePacket())
    {
      // check if ack packet:
      if (Comp_udp_rx.available() >= 3)
      {
        // read first three bytes
        _ReadPacket.packetReadBegin = true;
        Comp_udp_rx.readBytes(_ReadPacket.packetBuffer, 3);

        // check first three command bytes:
        if (charComp(_ReadPacket.packetBuffer, (byte *)"ack", 3))
        {
          _ReadPacket.packetAvailable = false;
          _ReadPacket.ackPacket = true;
          Comp_udp_rx.flush();
        }
        else
        {
          _ReadPacket.packetAvailable = true;
        }
      }
      else
      {
        _ReadPacket.packetAvailable = true;
        _ReadPacket.packetReadBegin = false;
      }
    }

    // send companion button press
    if ((millis() - _Timer) >= COMPANION_SEND_INTERVAL)
    {
      PressButton(_ConnectPage, _ConnectBank, 2);
      _Timer = millis();
    }
    if ((millis() - _CheckTimer) >= COMPANION_CONNECTION_TIMEOUT)
    {
      _Connected = _ReadPacket.ackPacket;
      _ReadPacket.ackPacket = false;
      if (!_Connected)
      {
        Comp_udp_rx.stop();
        Comp_udp_tx.stop();
        Comp_udp_rx.begin(_Ard_Port);
        Comp_udp_tx.begin(_Ard_Port);
      }
      _CheckTimer = millis();
    }
    return _Connected;
  }
  return false;
}

uint8_t COMP_UIP::GetCommand(byte *returnArr, uint8_t maxBytes)
{
  if (_ReadPacket.packetAvailable)
  {
    // determine total bytes available:
    uint16_t bytesAvailable = 0;
    if (_ReadPacket.packetReadBegin)
      bytesAvailable = Comp_udp_rx.available() + 3;
    else
      bytesAvailable = Comp_udp_rx.available();

    // check that provided buffer is large enough:
    if (bytesAvailable <= maxBytes)
    {
      // determine the correct way to transfer buffer:
      if (_ReadPacket.packetReadBegin)
      {
        uint16_t remBytes = Comp_udp_rx.available();
        Comp_udp_rx.readBytes(returnArr, remBytes);

        // shift all bytes by 3:
        for (uint16_t x = 0; x < remBytes; x++)
          returnArr[remBytes - 1 + 3 - x] = returnArr[remBytes - 1 - x];
        for (uint8_t x = 0; x < 3; x++)
          returnArr[x] = _ReadPacket.packetBuffer[x];

        _ReadPacket.packetAvailable = false;
        return bytesAvailable;
      }
      else
      {
        Comp_udp_rx.readBytes(returnArr, bytesAvailable);
        _ReadPacket.packetAvailable = false;
        return bytesAvailable;
      }
    }
  }
  return 0;
}

void COMP_UIP::PressButton(uint8_t Page, uint8_t Bank, uint8_t Dir)
{
  if (_Init)
  {
    if (Page > 0 && Page < 100 && Bank > 0 && Bank < 33)
    {
      const char BankStr[] = "bank-";
      const char SpaceStr[] = " ";
      char PageChar[3];
      char BankChar[3];
      {
        String Str = String(Page);
        Str.toCharArray(PageChar, 3);
      }
      {
        String Str = String(Bank);
        Str.toCharArray(BankChar, 3);
      }
      Comp_udp_tx.beginPacket(_Comp_IP, _Comp_Port);
      Comp_udp_tx.write(BankStr);
      if (Dir == 1)
      {
        char BankUpChar[] = "up ";
        Comp_udp_tx.write(BankUpChar);
      }
      else if (Dir == 0)
      {
        char BankDownChar[] = "down ";
        Comp_udp_tx.write(BankDownChar);
      }
      else
      {
        char BankPressChar[] = "press ";
        Comp_udp_tx.write(BankPressChar);
      }
      Comp_udp_tx.write(PageChar);
      Comp_udp_tx.write(SpaceStr);
      Comp_udp_tx.write(BankChar);
      Comp_udp_tx.endPacket();
    }
  }
}

#undef COMPANION_SEND_INTERVAL
#undef COMPANION_CONNECTION_TIMEOUT
#endif

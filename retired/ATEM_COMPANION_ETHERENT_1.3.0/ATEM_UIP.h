//ATEM_UIP REV 1.0.0
//.h
#ifndef ATEM_UIP_h
#define ATEM_UIP_h
#include "Arduino.h"

#include <SPI.h>
#include <UIPEthernet.h>
#include <UIPUdp.h>

EthernetUDP ATEM_UDP;

class ATEM_UIP {
  public:
    bool Run();
    void Setup(IPAddress IP, uint16_t Port) {
      _ATEM_IP = IP;
      _UDP_Port = Port;
      _Init = true;
      _Connected = false;
      ATEM_UDP.stop();
      _ConnectTimer = millis();
    }
    void Connect() {
      _Connected = false;
      _Init = true;
      ATEM_UDP.stop();
      _ConnectTimer = millis();
    }
    void Disconnect() {
      _Connected = false;
      _Init = false;
      ATEM_UDP.stop();
    }
    byte TallyInput[8];

  private:
    IPAddress _ATEM_IP = IPAddress(0, 0, 0, 0);
    uint16_t _UDP_Port = 0;
    bool _Connected = false;
    char _Buffer[96];
    uint16_t _SessionID;
    uint16_t _LastRemotePacketID;
    uint32_t _Timer;
    uint32_t _ConnectTimer;
    bool _PacketCount = false;
    bool _Init = false;
};

//.cpp
//#include "ATEM_UIP.h"
//#include "Arduino.h"

bool ATEM_UIP::Run() {
  if (_Init) {
    if (_Connected) {
      //Check for new Information and connection
      if ((millis() - _Timer) >= 1000) {
        if (!_PacketCount) _Connected = false;
        _PacketCount = false;
        _Timer = millis();
      }
      uint16_t PacketSize = ATEM_UDP.parsePacket();
      if (ATEM_UDP.available() && PacketSize) {
        ATEM_UDP.read(_Buffer, 12);
        uint16_t PacketLength = word(_Buffer[0] & B00000111, _Buffer[1]);
        _LastRemotePacketID = word(_Buffer[10], _Buffer[11]);
        uint8_t Command = _Buffer[0] & B11111000;
        bool CommandACK = Command & B00001000;
        if (PacketLength == PacketSize) {
          _PacketCount = true;
          if (PacketLength > 12) {
            bool ParsePacketDone = false;
            uint16_t IndexPointer = 12;
            while (IndexPointer < PacketLength && !ParsePacketDone) {
              ATEM_UDP.read(_Buffer, 2);
              uint16_t CommandLength = word(0, _Buffer[1]);
              if (CommandLength > 2 && CommandLength <= sizeof(_Buffer)) {
                ATEM_UDP.read(_Buffer, CommandLength - 2);
                char CommandString[] = {
                  _Buffer[2],
                  _Buffer[3],
                  _Buffer[4],
                  _Buffer[5], '\0'
                };
                if (!strcmp(CommandString, "TlIn")) {
                  TallyInput[0] = _Buffer[8];
                  TallyInput[1] = _Buffer[9];
                  TallyInput[2] = _Buffer[10];
                  TallyInput[3] = _Buffer[11];
                  TallyInput[4] = _Buffer[12];
                  TallyInput[5] = _Buffer[13];
                  TallyInput[6] = _Buffer[14];
                  TallyInput[7] = _Buffer[15];
                }
                IndexPointer += CommandLength;
              }
              else {
                ParsePacketDone = true;
                while (ATEM_UDP.available()) ATEM_UDP.read(_Buffer, sizeof(_Buffer));
              }
            }
          }
          if (CommandACK) {
            byte Answer[12] = {
              ((12 / 256) | B10000000), (12 % 256), 0x80,
              _SessionID, (_LastRemotePacketID / 256),
              (_LastRemotePacketID % 256), 0, 0, 0,
              0x41, 0, 0
            };
            ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            ATEM_UDP.write(Answer, 12);
            ATEM_UDP.endPacket();
          }
        }
        else {
          while (ATEM_UDP.available()) ATEM_UDP.read(_Buffer, sizeof(_Buffer));
        }
      }
    }
    else {
      //Connect
      if ((millis() - _ConnectTimer) >= 500) {
        ATEM_UDP.begin(_UDP_Port);
        {
          const byte ConnectByte[] = {
            0x10, 0x14, 0x53, 0xAB, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x3A,
            0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00
          };
          ATEM_UDP.beginPacket(_ATEM_IP, 9910);
          ATEM_UDP.write(ConnectByte, 20);
          ATEM_UDP.endPacket();
        }
        //    int B = 0;
        //    uint32_t Timer = millis();
        //    while (B != 20 && (millis() - Timer) < 10) {
        //      B = ATEM_UDP.parsePacket();
        //    }

        //Maybe this won't work:
        delay(5);
        if (ATEM_UDP.parsePacket() == 20) {
          //TODO: Reset ATEM Timer Connection
          _Connected = true;
          //      while (!ATEM_UDP.available());
          ATEM_UDP.read(_Buffer, 20);
          _SessionID = _Buffer[15];
          {
            const byte ConnectByte[] = {
              0x80, 0x0c, 0x53, 0xab, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x03, 0x00, 0x00
            };
            ATEM_UDP.beginPacket(_ATEM_IP, 9910);
            ATEM_UDP.write(ConnectByte, 12);
            ATEM_UDP.endPacket();
          }
          _Timer = millis();
          _PacketCount = true;
        }
        else ATEM_UDP.stop();
        _ConnectTimer = millis();
      }
    }
    return _Connected;
  }
  return false;
}

#endif

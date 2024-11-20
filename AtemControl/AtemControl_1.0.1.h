// AtemControl
/*
  changes from v1.0.0:
    - slight documentation changes
*/

//.h
#ifndef AtemControl_h
#define AtemControl_h

#define AtemDefaultPort 9910

// #define AtemSerialDebug

#include "Arduino.h"

#include <SPI.h>
#include <UIPEthernet.h>
#include <UIPUdp.h>

class AtemControl
{
public:
  /* setup():
  setup here. automatically starts connection as well */
  void setup(IPAddress, uint16_t);

  /* run():
  handles atem connection. run as often as possible.
  returns: true = connected, false = disconnected */
  bool run();
  /* connect():
  use to start or reset connection */
  void connect();
  /* disconnect():
  use to end connection.
  connection will stay disconnected until "start" or "connect" is called */
  void disconnect();

  /* doCut():
  send cut command on atem */
  void doCut();
  /* doAuto():
  send auto command on atem */
  void doAuto();
  /* previewInput():
  change preview input of atem */
  void previewInput(uint8_t);
  /* programInput():
  change program input on atem */
  void programInput(uint8_t);
  /* ftb():
  fade to black on atem */
  void ftb();

  /* send command packet to atem.
  atem api is needed for this.
  inputs(commandChar(4 bytes), commandBytes(up to 16 bytes)) */
  void sendCMD(char *, uint8_t *, uint8_t);
  /* tally():
  input which tally is needed. 
  returns tally state. updates each time packet received from atem */
  uint8_t tally(uint8_t);
private:
  IPAddress _atemIp;
  uint16_t _udpPort;

  bool _init = false;
  bool _connected = false;
  char _buffer[96];
  uint16_t _sessionId;
  uint16_t _lastPacketId;
  uint16_t _sendPacketId;
  uint16_t _last_TlIn_packetId;
  uint32_t _connectionTimer;
  bool _packetCount;

  uint8_t _atemTally[8];

  EthernetUDP _atemUDP;
};

//.cpp
void AtemControl::setup(IPAddress atemIp, uint16_t udpPort = 6565)
{
  _init = true;
  _connected = false;
  _atemIp = atemIp;
  _udpPort = udpPort;
  _connectionTimer = millis();
}

bool AtemControl::run()
{
  if (_init)
  {
    if (_connected)
    {
      if (millis() - _connectionTimer >= 1000)
      {
        if (!_packetCount)
        {
#ifdef AtemSerialDebug
          Serial.println("disconnected: timeout");
#endif
          _connected = false;
          _connectionTimer = millis();
          return false;
        }
        else
        {
          _packetCount = false;
          _connectionTimer = millis();
        }
      }
      // run packet check:
      uint16_t ParsedPacketSize = _atemUDP.parsePacket();
      if (_atemUDP.available() && ParsedPacketSize)
      {
        bool sendACK = false; // only send ack packet if we recieved correct packet

        _atemUDP.read(_buffer, 12);                                     // retrieve packet header
        uint16_t packetSize = word(_buffer[0] & B00000111, _buffer[1]); // get declared packet size to verify there were no errors
        uint16_t packetId = word(_buffer[10], _buffer[11]);
        uint8_t command = _buffer[0] & B11111000;
        bool commandACK = command & B00001000;
#ifdef AtemSerialDebug
        Serial.print("rPID: ");
        Serial.print(packetId);
        Serial.print(" ePID: ");
        Serial.println(_lastPacketId + 1);
#endif

        // increment packet id if we recieved worrecnt packet:
        if (packetId == _lastPacketId + 1)
        {
          _lastPacketId++;
          sendACK = true;
        }

        if (ParsedPacketSize == packetSize) // verify packet sizes match, otherwise flush buffer and exit
        {
          _packetCount = true; // successful packet, mark packetCount

          if (packetSize > 12 && packetId > _last_TlIn_packetId)
          {
            bool parsePacketDone = false;
            uint16_t indexPointer = 12; // we already read first 12 bytes
            while (indexPointer < packetSize && !parsePacketDone)
            {
              _atemUDP.read(_buffer, 2);
              uint16_t commandSize = word(0, _buffer[1]);
              if (commandSize > 2 && commandSize <= sizeof(_buffer))
              {
                _atemUDP.read(_buffer, commandSize - 2);
                char cmd_str[] =
                    {_buffer[2],
                     _buffer[3],
                     _buffer[4],
                     _buffer[5],
                     '\0'};
                if (!strcmp(cmd_str, "TlIn"))
                {
                  // if (packetId > _last_TlIn_packetId)
                  // {
                  sendACK = true;
                  _last_TlIn_packetId = packetId;
                  _lastPacketId = packetId;
                  _atemTally[0] = _buffer[8];
                  _atemTally[1] = _buffer[9];
                  _atemTally[2] = _buffer[10];
                  _atemTally[3] = _buffer[11];
                  _atemTally[4] = _buffer[12];
                  _atemTally[5] = _buffer[13];
                  _atemTally[6] = _buffer[14];
                  _atemTally[7] = _buffer[15];
#ifdef AtemSerialDebug
                  Serial.print("tally: ");
                  for (uint8_t x = 0; x < 8; x++)
                    Serial.print(_atemTally[x]);
                  Serial.println();
#endif
                  // }
                }
                indexPointer += commandSize;
              }
              else
              {
                parsePacketDone = true;
                while (_atemUDP.available())
                  _atemUDP.read(_buffer, sizeof(_buffer));
              }
            }
          }
          if (commandACK && sendACK)
          {
#ifdef AtemSerialDebug
            Serial.print("ACK: ");
            Serial.println(_lastPacketId);
#endif
            const byte answerACK[12] =
                {((12 / 256) | 0x80), (12 % 256), 0x80,
                 _sessionId, (_lastPacketId / 256),
                 (_lastPacketId % 256), 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00};

            _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
            _atemUDP.write(answerACK, 12);
            _atemUDP.endPacket();
          }
        }
        else
        {
          while (_atemUDP.available())
            _atemUDP.read(_buffer, sizeof(_buffer));
        }
      }
    }
    else
    {
      _atemUDP.stop();
      if (millis() - _connectionTimer >= 500) // don't spam reconnection
      {
#ifdef AtemSerialDebug
        Serial.print("connectAttempt: ");
#endif
        _atemUDP.begin(_udpPort);
        { // send connected packet:
          const byte connectPacket[] =
              {0x10, 0x14, 0x53, 0xAB, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x3A,
               0x00, 0x00, 0x01, 0x00, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x00};
          while (_atemUDP.available()) // flush udp buffer
            _atemUDP.read(_buffer, sizeof(_buffer));
          // send packet:
          _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
          _atemUDP.write(connectPacket, 20);
          _atemUDP.endPacket();
        }

        // listen for response:
        uint8_t udpParsePacket = _atemUDP.parsePacket();
        uint32_t parseTimeout = micros();
        while (micros() - parseTimeout < 10000 && udpParsePacket != 20) // keep looking for packet for 10(ms)
          udpParsePacket = _atemUDP.parsePacket();

        // check if response found
        if (udpParsePacket == 20)
        {
#ifdef AtemSerialDebug
          Serial.println("connected");
#endif
          // we assume this packet is from atem, no time to verify
          _connected = true;
          _sendPacketId = 1; // wierd but sendPacketId is expected to start from 1. wont work if it is 0
          _lastPacketId = 0;
          _last_TlIn_packetId = 0;

          // get session id from packet:
          _atemUDP.read(_buffer, 20);
          _sessionId = _buffer[15]; // this is where our session id is stored

          { // send conect response packet to confirm connection:
            const byte connectResponsePacket[] =
                {0x80, 0x0c, 0x53, 0xab, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
            // send packet:
            _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
            _atemUDP.write(connectResponsePacket, 12);
            _atemUDP.endPacket();
          }
          _packetCount = true;
        }
        else
        {
#ifdef AtemSerialDebug
          Serial.println("notConnected");
#endif
          // we did not get successful connection:
          _atemUDP.stop();
        }
        _connectionTimer = millis();
      }
    }
    return _connected;
  }
  else
    return false;
}
void AtemControl::connect()
{
  _init = true;
  _connected = false;
  _connectionTimer = millis();
}
void AtemControl::disconnect()
{
  _init = false;
  _connected = false;
  _atemUDP.stop();
}

void AtemControl::doCut()
{
  uint8_t cmdPacket[] =
      {0x00, 0x9d, 0x59, 0x99};
  sendCMD("DCut", cmdPacket, 4);
}
void AtemControl::doAuto()
{
  uint8_t cmdPacket[] =
      {0x00, 0xad, 0x59, 0x99};
  sendCMD("DAut", cmdPacket, 4);
}
void AtemControl::previewInput(uint8_t input)
{
  uint8_t cmdPacket[] =
      {0x00, 0x75, 0x00, input};
  sendCMD("CPvI", cmdPacket, 4);
}
void AtemControl::programInput(uint8_t input)
{
  uint8_t cmdPacket[] =
      {0x00, 0x75, 0x00, input};
  sendCMD("CPgI", cmdPacket, 4);
}
void AtemControl::ftb()
{
  uint8_t cmdPacket[] =
      {0x00, 0x5b, 0x49, 0x97};
  sendCMD("FtbA", cmdPacket, 4);
}

void AtemControl::sendCMD(char *cmd_str, uint8_t *cmd_packet, uint8_t cmd_bytes)
{
  if (_connected && _init) // only send command if we are connected. otherwise, arduino can freeze up
  {
    if (cmd_bytes <= 16)
    {

#ifdef AtemSerialDebug
      Serial.print("sendCMD: ");
      Serial.print(cmd_str);
      Serial.print(" bytes: ");
      for (uint8_t x = 0; x < cmd_bytes; x++)
      {
        Serial.print(cmd_packet[x]);
        Serial.print(" ");
      }
      Serial.println();
#endif

      const uint16_t packetSize = (20 + cmd_bytes);
      uint8_t answerPacket[packetSize];
      for (uint16_t x = 0; x < packetSize; x++) // clear entire buffer
        answerPacket[x] = 0;

      // declare packetSize:
      answerPacket[0] = packetSize / 256;
      answerPacket[1] = packetSize % 256;
      answerPacket[0] |= B00001000; // << api

      // declare session id and packet id:
      answerPacket[2] = 0x80; // << api
      answerPacket[3] = _sessionId;
      answerPacket[10] = _sendPacketId / 256;
      answerPacket[11] = _sendPacketId % 256;

      // declare cmd packet size:
      answerPacket[12] = (8 + cmd_bytes) / 256;
      answerPacket[13] = (8 + cmd_bytes) % 256;

      // cmd str:
      for (uint8_t x = 0; x < 4; x++)
        answerPacket[16 + x] = cmd_str[x];

      // command packet bytes:
      for (uint8_t x = 0; x < cmd_bytes; x++)
        answerPacket[20 + x] = cmd_packet[x];

      // send packet:
      _atemUDP.beginPacket(_atemIp, AtemDefaultPort);
      _atemUDP.write(answerPacket, packetSize);
      _atemUDP.endPacket();

      // increment send packet id:
      _sendPacketId++;
    }
  }
}
uint8_t AtemControl::tally(uint8_t input)
{
  if (input < 8 && _init)
    return _atemTally[input];
  return 0;
}

#endif

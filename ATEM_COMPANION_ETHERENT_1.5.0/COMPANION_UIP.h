//COMAPNION_UIP REV 1.0.4
//.h
#ifndef COMPANION_UIP_h
#define COMPANION_UIP_h

#include "Arduino.h"

#include <SPI.h>
#include <UIPEthernet.h>
#include <UIPUdp.h>

EthernetUDP COMP_TX;
EthernetUDP COMP_RX;

class COMP_UIP {
  public:
    //0 = Disconnected, 1 = Connected, 2 = ConnectionSleep
    uint8_t Run();
    void Setup(IPAddress COMP_IP, uint16_t RX_PORT, uint16_t TX_PORT) {
      _Init = true;
      _COMP_IP = COMP_IP;
      _RX_PORT = RX_PORT;
      _TX_PORT = TX_PORT;
      _Packet = false;
      _Connected = false;
      _ConnectionSleepTimer = millis();
      _Timer = millis();
      _CheckTimer = millis();
      COMP_TX.stop();
      COMP_RX.stop();
      COMP_TX.begin(_TX_PORT);
      COMP_RX.begin(_RX_PORT);
    }
    void Connect() {
      _Init = true;
      _Packet = false;
      _Connected = false;
      _ConnectionSleepTimer = millis();
      _Timer = millis();
      _CheckTimer = millis();
      COMP_TX.stop();
      COMP_RX.stop();
      COMP_TX.begin(_TX_PORT);
      COMP_RX.begin(_RX_PORT);
    }
    void Disconnect() {
      _Init = false;
      _Packet = false;
      _Connected = false;
      COMP_TX.stop();
      COMP_RX.stop();
    }
    void ConnectionSleep(uint16_t Seconds) {
      _ConnectionSleepDelay = (uint32_t)Seconds * 1000;
    }
    uint16_t SleepTimer() {
      if (_Connected && !_ConnectionSleep)
        return ((_ConnectionSleepDelay - (millis() - _ConnectionSleepTimer)) / 1000);
      else return 0;
    }
    void ResetSleep() {
      _ConnectionSleep = false;
      _ConnectionSleepTimer = millis();
    }

    void SetConnectButton(uint8_t Page, uint8_t Bank) {
      if (Page > 0 && Page < 100 && Bank > 0 && Bank < 33) {
        _ConnectPage = Page;
        _ConnectBank = Bank;
      }
    }

    void PressButton(uint8_t Page, uint8_t Bank, bool Dir);

  private:
    IPAddress _COMP_IP = IPAddress(0, 0, 0, 0);
    uint16_t _COMP_Port = 51235;//Default Companion UDP Port. This default value Caused so many issues :(
    uint16_t _RX_PORT = 8000;
    uint16_t _TX_PORT = 8888;
    uint8_t _ConnectPage = 15;
    uint8_t _ConnectBank = 1;
    bool _Init = false;
    bool _Packet = false;
    bool _Connected = false;
    uint32_t _Timer = 0;
    uint32_t _CheckTimer = 0;
    uint32_t _ConnectionSleepDelay = (60 * 60 * 1000); //One Hour, Default
    uint32_t _ConnectionSleepTimer = 0;
    bool _ConnectionSleep = false;
};

//.cpp
//#include "COMPANION_UIP.h"
//#include "Arduino.h"

uint8_t COMP_UIP::Run() {
  if (_Init) {
    if (!_ConnectionSleep) {
      if (_ConnectionSleepDelay) {
        if ((millis() - _ConnectionSleepTimer) >= _ConnectionSleepDelay) {
          _Connected = false;
          _ConnectionSleep = true;
          return 2;
        }
      }
      if (COMP_RX.parsePacket()) {
        _Packet = true;
        char TempBuffer[5];
        while (COMP_RX.available()) COMP_RX.read(TempBuffer, 5);
      }
      if ((millis() - _Timer) >= 300) {
        PressButton(_ConnectPage, _ConnectBank, false);
        _Timer = millis();
      }
      if ((millis() - _CheckTimer) >= 1000) {
        _Connected = _Packet;
        _Packet = false;
        if (!_Connected) {
          COMP_TX.stop();
          COMP_RX.stop();
          COMP_TX.begin(_TX_PORT);
          COMP_RX.begin(_RX_PORT);
        }
        _CheckTimer = millis();
      }
      return _Connected;
    }
    else return 2;
  }
  return 0;
}

void COMP_UIP::PressButton(uint8_t Page, uint8_t Bank, bool Dir) {
  if (_Init) {
    if (Page > 0 && Page < 100 && Bank > 0 && Bank < 33) {
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
      COMP_TX.beginPacket(_COMP_IP, _TX_PORT);
      COMP_TX.write(BankStr);
      if (Dir) {
        char BankUpChar[] = "up ";
        COMP_TX.write(BankUpChar);
      }
      else {
        char BankDownChar[] = "down ";
        COMP_TX.write(BankDownChar);
      }
      COMP_TX.write(PageChar);
      COMP_TX.write(SpaceStr);
      COMP_TX.write(BankChar);
      COMP_TX.endPacket();
    }
  }
}

#endif

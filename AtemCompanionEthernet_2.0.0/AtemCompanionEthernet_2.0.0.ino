#define RTX_DEVICE_NAME "ETHER"
#define RTX_SOFTWARE_VERSION 2, 0, 0
#define RTX_DEVICE_IP 1

#define RTX_EEPROM_SECTORS 3 // 24 bytes
#define RTX_DEBUG_SECTORS 2  // 16 bytes

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\FSCC\RTX Systems\RTX V2\ATEM_COMPANION_ETHERNET\AtemCompanionEthernet_2.0.0\include.h"

//

//

// define global port values:. Both are simply receive ports, doesn't matter what they are, as long as they dont conflict.
#define UDP_General_Port 8888
#define UDP_ATEM_Port 6565
#define MAC_Arduino 0x98, 0xA6, 0xAA, 0x15, 0xE8, 0xD9

// Makes it a little easier to get buffer values:
#define CONFIG_BUFF(ADD) (RTX_PROTOCAL.RTX_EEPROM_BUFFER[ADD])
#define DEBUG_BUFF(ADD) (RTX_PROTOCAL.RTX_DEBUG_BUFFER[ADD])

namespace network
{
  inline void init();
  inline void handle();
  static void ethernetSetup();

  // Setup Companion and ATEM Objects
  AtemControl ATEM;
  COMP_UIP Companion;

  Par_bool ATEM_Connected = false;
  Par_bool Companion_Connected = false;

  Par_bool NetworkRunning = false;

  /*
    I know something like the atem-constellation exists,
    but for now, we only worry about up to 8 inputs:
  */
  Par_uint8_t ATEM_Tallies[8]; // all should be assumed '0'

  bool NetworkInit = false;
}

namespace rtx_send
{
  inline void init();
  void subscribe(uint8_t, bool);
  inline void handle();
  void send_once(uint8_t);

  bool SubscribedDevices[8];
  uint32_t ConnectTimer[8];
  uint32_t SendTimer[8];
}

void setup()
{
  network::init();
  rtx_send::init();
}

void loop()
{
  network::handle();
  rtx_send::handle();

  // update Debug registers:
  {
    // Update Total Run Time in Debug Registers:
    {
      uint32_t m = millis();
      DEBUG_BUFF(8) = (m / 3600000L);
      DEBUG_BUFF(9) = ((m % 3600000L) / 60000L);
    }
  }

  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR)
  {
  }
  else
  {
  }

  // note, tally subscription service always runs:
}

inline void rtx_send::init()
{
  for (uint8_t x = 0; x < sizeof(SubscribedDevices) / sizeof(bool); x++)
    SubscribedDevices[x] = false;
}

void rtx_send::subscribe(uint8_t slave_ip, bool sub_command = true)
{
  if (slave_ip < 8)
  {
    if (sub_command)
    {
      if (SubscribedDevices[slave_ip]) // device already subscribed
      {
        ConnectTimer[slave_ip] = millis();
      }
      else // new subscription
      {
        SubscribedDevices[slave_ip] = true;
        ConnectTimer[slave_ip] = millis();
        send_once(slave_ip);
      }
    }
  }
}

inline void rtx_send::handle()
{
  // TODO: update a debug register with subscribed devices:
}

void rtx_send::send_once(uint8_t slave_ip = 255)
{
  if (slave_ip < 8) // target one device
  {
  }
  else // target all subscribed devices
  {
  }
}

inline void network::init()
{
  for (uint8_t x = 0; x < sizeof(ATEM_Tallies) / sizeof(Par_uint8_t); x++)
    ATEM_Tallies[x].var = 0;
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR)
    ethernetSetup();
}

inline void network::handle()
{
  // all Ethernet instances should keep track of their own initialization
  network::Companion_Connected.var = network::Companion.Run();
  network::ATEM_Connected.var = network::ATEM.run();
  for (uint8_t x = 0; x < sizeof(network::ATEM_Tallies) / sizeof(Par_uint8_t); x++)
    network::ATEM_Tallies[x].var = network::ATEM.tally(x);

  // TODO: check for tally changes, send over rtx if necessary.

  // Update Registers:
  // update debug Ethernet Connection:
  {
    DEBUG_BUFF(0) = (network::ATEM_Connected.var << 1);
    DEBUG_BUFF(0) |= (network::Companion_Connected.var << 0);
  }
  // update tally debug registers:
  {
    DEBUG_BUFF(1) = 0; // PRG
    DEBUG_BUFF(2) = 0; // PRE
    for (uint8_t x = 0; x < sizeof(network::ATEM_Tallies) / sizeof(Par_uint8_t); x++)
    {
      DEBUG_BUFF(1) |= ((bool)(network::ATEM_Tallies[x].var & B00000001)) << x; // PRG
      DEBUG_BUFF(2) |= ((bool)(network::ATEM_Tallies[x].var & B00000010)) << x; // PRE
    }
  }

  // check network reset commands:
  if (!RTX_PROTOCAL.RTX_EEPROM_ERROR) // << since this rely's on Config buffer, check only when no EEPROM ERROR's
  {
    // companion disabled:
    if (!(CONFIG_BUFF(7) & B1))
      network::Companion.Disconnect();
    // ATEM disabled:
    if (!(CONFIG_BUFF(7) & B10))
      network::ATEM.disconnect();
  }

  // reset Ethernet Command:
  if (DEBUG_BUFF(7) & B1)
  {
    DEBUG_BUFF(7) &= B11111110;
    if (!RTX_PROTOCAL.RTX_EEPROM_ERROR)
      ethernetSetup();
  }

  // Halt Ethernet:
  if (DEBUG_BUFF(7) & B10)
  {
    DEBUG_BUFF(7) &= B11111101;

    network::Companion.Disconnect();
    network::Companion_Connected.var = false;

    network::ATEM.disconnect();
    network::ATEM_Connected.var = false;
  }
}

static void network::ethernetSetup()
{
  byte ard_mac[] = {MAC_Arduino};
  IPAddress ard_ip(CONFIG_BUFF(0), CONFIG_BUFF(1), CONFIG_BUFF(2), CONFIG_BUFF(3));

  Ethernet.begin(ard_mac, ard_ip);

  // setup companion:
  if (CONFIG_BUFF(7) & B1)
  {
    IPAddress companion_ip(CONFIG_BUFF(8), CONFIG_BUFF(9), CONFIG_BUFF(10), CONFIG_BUFF(11));
    uint16_t companion_port = (CONFIG_BUFF(13) | (CONFIG_BUFF(12) << 8));
    network::Companion.SetConnectButton(CONFIG_BUFF(14), CONFIG_BUFF(15));
    network::Companion.Setup(companion_ip, companion_port, UDP_General_Port);
  }
  else
  {
    network::Companion.Disconnect();
    network::Companion_Connected.var = false;
  }

  // setup ATEM connection:
  if (CONFIG_BUFF(7) & B10)
  {
    IPAddress ATEM_ip(CONFIG_BUFF(16), CONFIG_BUFF(17), CONFIG_BUFF(18), CONFIG_BUFF(19));
    network::ATEM.setup(ATEM_ip, AtemDefaultPort);
  }
  else
  {
    network::ATEM.disconnect();
    network::ATEM_Connected.var = false;
  }
}
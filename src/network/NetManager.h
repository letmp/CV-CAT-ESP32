#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <ESPmDNS.h>
#include <ETH.h>

#include <network/NetServiceMqtt.h>
#include <network/NetServiceHttp.h>
#include <network/NetConstants.h>
#include <network/NetConfig.h>

#define ETH_ADDR 0		 // I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_POWER_PIN -1 // Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_MDC_PIN 23	 // Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDIO_PIN 18	 // Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_TYPE ETH_PHY_LAN8720

#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT // ETH_CLOCK_GPIO0_IN

class NetManager
{

private:

	NetConfig mNetConfig;
	NetServiceHttp mNetServiceHttp;
	NetServiceMqtt mNetServiceMqtt;

	bool initWifiAP();
	bool initWifiSTA();
	bool initETH();
	void initMdns();

public:
	NetManager(HardwareManager& hardwareManager);
	void begin();
	void loop();
};

#endif
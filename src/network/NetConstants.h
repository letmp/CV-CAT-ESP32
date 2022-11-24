#ifndef NET_CONSTANTS_H
#define NET_CONSTANTS_H

#include <Arduino.h>

class NetConstants
{
public:
	static const int CON_TIMEOUT;
	static const int PORT_HTTP;
	static const int PORT_MQTT;
	static const String DEVICENAME;
	static const String HOSTNAME;
	static const String WIFI_SSID;
	static const String WIFI_PWD;
	static const String WIFI_IP_STATIC;
	static const String WIFI_GATEWAY_STATIC;
	static const String ETH_IP_STATIC;
	static const String ETH_GATEWAY_STATIC;
	static const String NETCONFIG;
};

#endif
#ifndef NETWORK_DATA_H
#define NETWORK_DATA_H

#include <Arduino.h>
#include <Streaming.h>

#include <network/NetConstants.h>
#include <persistence/PersistenceUtils.h>

class NetConfig
{

private:
	PersistenceUtils mPersistenceUtils;
    bool loadWifiConfig();
	bool loadEthConfig();

public:
    String uniqueHostname;
	String wifiSSID;
	String wifiPWD;

	IPAddress wifiIp;
	IPAddress wifiIpGateway;
	IPAddress ethIp;
	IPAddress ethIpGateway;

    bool hasWifiConfig;
    bool hasEthConfig;
    bool hasStaticEthAddress;
    bool hasStaticWifiAddress;

	NetConfig();
    void writeWifiConfig();
    void writeEthConfig();
	
};

#endif
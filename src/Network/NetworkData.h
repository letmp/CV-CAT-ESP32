#ifndef NETWORK_DATA_H
#define NETWORK_DATA_H

#include <Arduino.h>
#include <Network/ConstantProvider.h>
#include <Persistence/PersistenceManager.h>
#include <Streaming.h>

class NetworkData
{

private:
	PersistenceManager &mPersistenceManager;
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

	NetworkData(PersistenceManager& pm);
    void writeWifiConfig();
    void writeEthConfig();
	
};

#endif
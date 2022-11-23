#include "NetworkManager.h"

NetworkManager::NetworkManager(NetworkData& nd, HardwareManager& hm) :
								   mNetworkData{nd},
								   mWebserviceManager(mNetworkData, hm),
								   mMqttManager(mNetworkData, hm)
{
}

void NetworkManager::begin()
{
	
	if (mNetworkData.hasWifiConfig)
	{
		initWifiSTA();
	}
	else
	{
		initWifiAP();
	}
	initETH();
	initMdns();

	mWebserviceManager.begin();
	MDNS.addService("http", "tcp", ConstantProvider::PORT_HTTP);

	// if(networkManager.isMaster) networkManager.startBroker();
	//MDNS.addService("mqtt", "tcp", ConstProvider::PORT_MQTT);
  // networkManager.initClients();
}



bool NetworkManager::initWifiAP()
{
	Serial << endl << "--- Initializing WIFI as Access Point ---" << endl;

	WiFi.softAP(mNetworkData.uniqueHostname.c_str(), NULL);

	Serial << "IP Address [" << WiFi.softAPIP();
	Serial << "] / SSID [" << mNetworkData.uniqueHostname << "]" << endl;

	mNetworkData.wifiIp = WiFi.softAPIP();

	return true;
}

bool NetworkManager::initWifiSTA()
{
	Serial << endl << "--- Initializing WIFI as Station ---" << endl;

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(mNetworkData.uniqueHostname.c_str());

	if (mNetworkData.hasStaticWifiAddress && !WiFi.config(mNetworkData.wifiIp, mNetworkData.wifiIpGateway, IPAddress(255,255,0,0)))
	{
		Serial.println("Failed to initialize with internal stored IPs");
		return false;
	}

	WiFi.begin(mNetworkData.wifiSSID.c_str(), mNetworkData.wifiPWD.c_str());

	Serial << "Connecting";
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis;

	while (WiFi.status() != WL_CONNECTED)
	{
		currentMillis = millis();
		if (currentMillis - previousMillis >= ConstantProvider::CON_TIMEOUT)
		{
			Serial.println("Failed to connect.");
			return false;
		}
		Serial << '.';
		delay(500);
	}

	Serial << endl << "Connected to SSID [" << mNetworkData.wifiSSID << "]" << endl;
	if (!mNetworkData.hasStaticWifiAddress)
	{
		mNetworkData.wifiIp = WiFi.localIP();
		mNetworkData.wifiIpGateway = WiFi.gatewayIP();
		Serial << "Retrieved ";
	}

	Serial << "IP Address [" << mNetworkData.wifiIp;
	Serial << "] / Gateway IP [" << mNetworkData.wifiIpGateway << "]" << endl;

	return true;
}

bool NetworkManager::initETH()
{
	Serial << endl << "--- Initializing ETH ---" << endl;

	ETH.setHostname(mNetworkData.uniqueHostname.c_str());
	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

	if (mNetworkData.hasStaticEthAddress && !ETH.config(mNetworkData.ethIp, mNetworkData.ethIpGateway, IPAddress(255,255,0,0)))
	{
		Serial.println("Failed to initialize with internal stored IPs");
		return false;
	}

	Serial << "Connecting";
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis;

	while (!((uint32_t)ETH.localIP())) // wait for IP
	{
		currentMillis = millis();
		if (currentMillis - previousMillis >= ConstantProvider::CON_TIMEOUT)
		{
			Serial << endl << "Failed to connect" << endl;
			return false;
		}
		Serial << '.';
		delay(500);
	}
	
	Serial << endl;

	if (!mNetworkData.hasStaticEthAddress)
	{
		mNetworkData.ethIp = ETH.localIP();
		mNetworkData.ethIpGateway = ETH.gatewayIP();
		Serial << "Retrieved ";
	}

	Serial << "IP Address [" << mNetworkData.ethIp;
	Serial << "] / Gateway IP [" << mNetworkData.ethIpGateway << "]" << endl;

	return true;
}

void NetworkManager::initMdns()
{
	Serial << endl << "--- Starting mDNS";
	while (!MDNS.begin(mNetworkData.uniqueHostname.c_str()))
	{
		Serial << '.';
		delay(1000);
	}
	Serial << " ---" << endl
		   << "Hostname [" << mNetworkData.uniqueHostname << "]" << endl;
}



void NetworkManager::loop()
{
	mMqttManager.loop();
}
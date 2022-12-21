#include "NetManager.h"

NetManager::NetManager(HardwareManager &hm) : mNetServiceHttp(mNetConfig, hm),
											  mNetServiceMqtt(mNetConfig, hm)
{
}

void NetManager::begin()
{
	initEthEventListener();

	mNetConfig.init();
	bool staSuccess = false;
	if (mNetConfig.hasWifiConfig)
		staSuccess = initWifiSTA();
	if (!staSuccess)
		initWifiAP();

	initETH();
	initMdns();

	mNetServiceHttp.begin();
	MDNS.addService("http", "tcp", NetConstants::PORT_HTTP);

	mNetServiceMqtt.begin();
	MDNS.addService("mqtt", "tcp", NetConstants::PORT_MQTT);
	MDNS.addServiceTxt("mqtt", "tcp","eth", mNetConfig.ethIp.toString());
	MDNS.addServiceTxt("mqtt", "tcp","wifi", mNetConfig.wifiIp.toString());

	delay(1000);
	mNetServiceMqtt.findRemoteBrokers();
}

bool NetManager::initWifiAP()
{
	Serial << endl << "--- Initializing WIFI as Access Point ---" << endl;

	WiFi.softAP(mNetConfig.uniqueHostname.c_str(), NULL);

	Serial << "IP Address [" << WiFi.softAPIP();
	Serial << "] / SSID [" << mNetConfig.uniqueHostname << "]" << endl;

	mNetConfig.wifiIp = WiFi.softAPIP();

	return true;
}

bool NetManager::initWifiSTA()
{
	Serial << endl << "--- Initializing WIFI as Station ---" << endl;

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(mNetConfig.uniqueHostname.c_str());

	if (mNetConfig.hasStaticWifiAddress && !WiFi.config(mNetConfig.wifiIp, mNetConfig.wifiIpGateway, IPAddress(255, 255, 0, 0)))
	{
		Serial.println("Failed to initialize with internal stored IPs");
		return false;
	}

	WiFi.begin(mNetConfig.wifiSSID.c_str(), mNetConfig.wifiPWD.c_str());

	Serial << "Connecting";
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis;

	while (WiFi.status() != WL_CONNECTED)
	{
		currentMillis = millis();
		if (currentMillis - previousMillis >= NetConstants::CON_TIMEOUT)
		{
			Serial.println("Failed to connect.");
			return false;
		}
		Serial << '.';
		delay(500);
	}

	Serial << endl << "Connected to SSID [" << mNetConfig.wifiSSID << "]" << endl;
	if (!mNetConfig.hasStaticWifiAddress)
	{
		mNetConfig.wifiIp = WiFi.localIP();
		mNetConfig.wifiIpGateway = WiFi.gatewayIP();
		Serial << "DHCP assigned ";
	}
	else
		Serial << "Static ";

	Serial << "IP Address [" << mNetConfig.wifiIp;
	Serial << "] / Gateway IP [" << mNetConfig.wifiIpGateway << "]" << endl;

	return true;
}

void NetManager::initEthEventListener()
{
	WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
										 {
		switch (event) {
		case ARDUINO_EVENT_ETH_START:
			Serial << "ETH Started" << endl;
			break;
		case ARDUINO_EVENT_ETH_CONNECTED:
			Serial << "ETH Connected" << endl;
			break;
		case ARDUINO_EVENT_ETH_GOT_IP:
			Serial << "ETH MAC: " << ETH.macAddress();
			if (ETH.fullDuplex()) Serial<< ", FULL_DUPLEX";
			Serial << ", " << ETH.linkSpeed() << "Mbps" << endl;
			break;
		case ARDUINO_EVENT_ETH_DISCONNECTED:
			Serial << "ETH Disonnected" << endl;
			break;
		case ARDUINO_EVENT_ETH_STOP:
			Serial << "ETH Stopped" << endl;
			break;
		default:
			break;
		}
	});
}

bool NetManager::initETH()
{
	Serial << endl << "--- Initializing ETH ---" << endl;

	ETH.setHostname(mNetConfig.uniqueHostname.c_str());
	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

	if (mNetConfig.hasStaticEthAddress && !ETH.config(mNetConfig.ethIp, mNetConfig.ethIpGateway, IPAddress(255, 255, 255, 0)))
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
		if (currentMillis - previousMillis >= NetConstants::CON_TIMEOUT)
		{
			Serial << endl << "Failed to connect" << endl;
			return false;
		}
		Serial << '.';
		delay(500);
	}

	Serial << endl;

	if (!mNetConfig.hasStaticEthAddress)
	{
		mNetConfig.ethIp = ETH.localIP();
		mNetConfig.ethIpGateway = ETH.gatewayIP();
		Serial << "DHCP assigned ";
	}
	else
		Serial << "Static ";

	Serial << "IP Address [" << mNetConfig.ethIp;
	Serial << "] / Gateway IP [" << mNetConfig.ethIpGateway << "]" << endl;

	return true;
}

void NetManager::initMdns()
{
	Serial << endl<< "--- Starting mDNS";
	while (!MDNS.begin(mNetConfig.uniqueHostname.c_str()))
	{
		Serial << '.';
		delay(1000);
	}
	Serial << " ---" << endl << "Hostname [" << mNetConfig.uniqueHostname << "]" << endl;
}

void NetManager::loop()
{
	mNetServiceMqtt.loop();
}
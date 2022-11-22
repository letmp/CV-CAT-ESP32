#include "NetworkManager.h"
#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

NetworkManager::NetworkManager() : mIpAddressSubnet(255, 255, 255, 0),
								   mAsyncWebServer(80),
								   mBroker(MQTT_PORT),
								   mClientState(&mBroker),
								   mClientDataTransfer(&mBroker)
{
}

void NetworkManager::begin()
{
	setUniqueHostname();

	bool hasWifiConfig = loadWifiConfig();
	if (hasWifiConfig)
	{
		initWifiSTA();
	}
	else
	{
		initWifiAP();
	}

	loadEthConfig();
	initETH();

	initMdns();
	startWebServer(hasWifiConfig);
}

void NetworkManager::setUniqueHostname()
{
	char uniqueHostname[23];
	snprintf(uniqueHostname, 23, DEVICE_NAME "-%llX", ESP.getEfuseMac());

	Serial << endl
		   << "--- Setting unique hostname [" << uniqueHostname << "] ---" << endl;
	mUniqueHostname = uniqueHostname;
}

bool NetworkManager::loadWifiConfig()
{
	Serial << endl << "--- Loading WIFI Config---" << endl;

	String wifiSSID = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_SSID);
	String wifiPassword = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_PWD);
	String wifiIP = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_STATIC_WIFI_IP);
	String wifiGateway = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_STATIC_WIFI_GATEWAY);

	if (wifiSSID == "")
	{
		Serial << "Undefined SSID. Starting Access Point." << endl;
		return false;
	}
	mWifiSSID = wifiSSID;
	mWifiPassword = wifiPassword;

	if (wifiIP != "" || wifiGateway != "")
	{
		mHasStaticWifiAddress =
			mIpAddressWifi.fromString(wifiIP.c_str()) &&
			mIpAddressWifiGateway.fromString(wifiGateway.c_str());
	}
	return true;
}

bool NetworkManager::loadEthConfig()
{
	Serial << endl << "--- Loading ETH Config---" << endl;

	String ethIP = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_STATIC_ETH_IP);
	String ethGateway = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_STATIC_ETH_GATEWAY);

	if (ethIP != "" || ethGateway != "")
	{
		mHasStaticEthAddress =
			mIpAddressEth.fromString(ethIP.c_str()) &&
			mIpAddressEthGateway.fromString(ethGateway.c_str());
	}

	return true;
}

bool NetworkManager::initWifiAP()
{
	Serial << endl << "--- Initializing WIFI as Access Point ---" << endl;

	WiFi.softAP(mUniqueHostname.c_str(), NULL);

	Serial << "IP Address [" << WiFi.softAPIP();
	Serial << "] / SSID [" << mUniqueHostname << "]" << endl;

	mIpAddressWifi = WiFi.softAPIP();

	return true;
}

bool NetworkManager::initWifiSTA()
{
	Serial << endl << "--- Initializing WIFI as Station ---" << endl;

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(mUniqueHostname.c_str());

	if (mHasStaticWifiAddress && !WiFi.config(mIpAddressWifi, mIpAddressWifiGateway, mIpAddressSubnet))
	{
		Serial.println("Failed to initialize with internal stored IPs");
		return false;
	}

	WiFi.begin(mWifiSSID.c_str(), mWifiPassword.c_str());

	Serial << "Connecting";
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis;

	while (WiFi.status() != WL_CONNECTED)
	{
		currentMillis = millis();
		if (currentMillis - previousMillis >= TIMEOUT_NETWORK)
		{
			Serial.println("Failed to connect.");
			return false;
		}
		Serial << '.';
		delay(500);
	}

	Serial << endl << "Connected to SSID [" << mWifiSSID << "]" << endl;
	if (!mHasStaticWifiAddress)
	{
		mIpAddressWifi = WiFi.localIP();
		mIpAddressWifiGateway = WiFi.gatewayIP();
		Serial << "Retrieved ";
	}

	Serial << "IP Address [" << mIpAddressWifi;
	Serial << "] / Gateway IP [" << mIpAddressWifiGateway << "]" << endl;

	return true;
}

bool NetworkManager::initETH()
{
	Serial << endl << "--- Initializing ETH ---" << endl;

	ETH.setHostname(mUniqueHostname.c_str());
	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

	if (mHasStaticEthAddress && !ETH.config(mIpAddressEth, mIpAddressEthGateway, mIpAddressSubnet))
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
		if (currentMillis - previousMillis >= TIMEOUT_NETWORK)
		{
			Serial << endl << "Failed to connect" << endl;
			return false;
		}
		Serial << '.';
		delay(500);
	}
	
	Serial << endl;

	if (!mHasStaticEthAddress)
	{
		mIpAddressEth = ETH.localIP();
		mIpAddressEthGateway = ETH.gatewayIP();
		Serial << "Retrieved ";
	}

	Serial << "IP Address [" << mIpAddressEth;
	Serial << "] / Gateway IP [" << mIpAddressEthGateway << "]" << endl;

	return true;
}

void NetworkManager::initMdns()
{
	Serial << endl << "--- Starting mDNS";
	while (!MDNS.begin(mUniqueHostname.c_str()))
	{
		Serial << '.';
		delay(1000);
	}
	Serial << " ---" << endl
		   << "Hostname [" << mUniqueHostname << "]" << endl;
}

void NetworkManager::startWebServer(bool hasWifiConfig)
{
	Serial << endl << "--- Starting WebServer ---" << endl;

	mAsyncWebServer.on("/", HTTP_GET, std::bind(&NetworkManager::handleGetNetconfig, this, std::placeholders::_1));
	mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetworkManager::handlePostNetconfig, this, std::placeholders::_1));
	mAsyncWebServer.serveStatic("/", SPIFFS, "/");

	mAsyncWebServer.on("/io/", HTTP_GET, std::bind(&NetworkManager::handleGetIO, this, std::placeholders::_1));
	mAsyncWebServer.on("/io/switch", HTTP_GET, std::bind(&NetworkManager::handleGetIOSwitch, this, std::placeholders::_1));
	mAsyncWebServer.serveStatic("/io/", SPIFFS, "/");
	

	if (hasWifiConfig)
		AsyncElegantOTA.begin(&mAsyncWebServer);

	mAsyncWebServer.begin();
	MDNS.addService("http", "tcp", 80);
	Serial << "http:\\\\" << mUniqueHostname << ".local" << endl;
}

void NetworkManager::handleGetIO(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/io.html", "text/html");
}
void NetworkManager::handleGetIOSwitch(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/io.html", "text/html");
}

void NetworkManager::handleGetNetconfig(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/netconfig.html", String(), false, [this](const String &var) -> String { return this->processTemplateNetconfig(var); });
}

String NetworkManager::processTemplateNetconfig(const String &var)
{
	if (var == PARAM_NETCONFIG)
	{
		String config = "<b>Your current network settings</b> <br/>";
		config += "WIFI IP: " + mIpAddressWifi.toString() + " / WIFI GATEWAY: " + mIpAddressWifiGateway.toString() + "<br/>";
		config += "ETH IP: " + mIpAddressEth.toString() + " / ETH GATEWAY: " + mIpAddressEthGateway.toString();
		return F(config.c_str());
	}

	if (var == PARAM_WIFI_SSID)
		return F(mWifiSSID.c_str());
	if (var == PARAM_WIFI_PWD)
		return F(mWifiPassword.c_str());

	String ipAddress;
	if (var == PARAM_STATIC_WIFI_IP && mHasStaticWifiAddress)
	{
		ipAddress = mIpAddressWifi.toString();
		if (ipAddress != "0.0.0.0")
			return F(ipAddress.c_str());
	}
	if (var == PARAM_STATIC_WIFI_GATEWAY && mHasStaticWifiAddress)
	{
		ipAddress = mIpAddressWifiGateway.toString();
		if (ipAddress != "0.0.0.0")
			return F(ipAddress.c_str());
	}
	if (var == PARAM_STATIC_ETH_IP && mHasStaticEthAddress)
	{
		ipAddress = mIpAddressEth.toString();
		if (ipAddress != "0.0.0.0")
			return F(ipAddress.c_str());
	}
	if (var == PARAM_STATIC_ETH_GATEWAY && mHasStaticEthAddress)
	{
		ipAddress = mIpAddressEthGateway.toString();
		if (ipAddress != "0.0.0.0")
			return F(ipAddress.c_str());
	}

	return String();
}

void NetworkManager::handlePostNetconfig(AsyncWebServerRequest *request)
{
	int params = request->params();
	for (int i = 0; i < params; i++)
	{
		AsyncWebParameter *p = request->getParam(i);
		if (p->isPost())
		{
			writeParameterToSPIFFS(p, PARAM_WIFI_SSID);
			writeParameterToSPIFFS(p, PARAM_WIFI_PWD);
			writeParameterToSPIFFS(p, PARAM_STATIC_WIFI_IP);
			writeParameterToSPIFFS(p, PARAM_STATIC_WIFI_GATEWAY);
			writeParameterToSPIFFS(p, PARAM_STATIC_ETH_IP);
			writeParameterToSPIFFS(p, PARAM_STATIC_ETH_GATEWAY);
		}
	}
	request->send(200, "text/html", "Done. ESP will restart, connect to your router and go to <a href=\"http://" + mUniqueHostname + ".local\">" + mUniqueHostname + ".local</a>");
	delay(3000);
	ESP.restart();
}

void NetworkManager::writeParameterToSPIFFS(AsyncWebParameter *p, String parameter)
{
	if (p->name() == parameter)
	{
		mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + parameter, p->value().c_str());
	}
}

void NetworkManager::startBroker()
{
	Serial << endl << "--- Starting local broker --- " << endl;
	mBroker.begin();
	MDNS.addService("mqtt", "tcp", MQTT_PORT);
}

IPAddress NetworkManager::findStatusBroker()
{
	// while(mdns_init()!= ESP_OK){ Serial << '.'; delay(1000); }
	IPAddress brokerIp;
	Serial << endl
		   << " --- Resolving Broker IP";
	while (brokerIp.toString() == "0.0.0.0")
	{
		Serial << '.';
		brokerIp = MDNS.queryHost(DEVICE_NAME);
		delay(1000);
	}
	int nrOfServices = MDNS.queryService("mqtt", "tcp");
	if (nrOfServices == 0)
		Serial.println("No services were found.");
	else
	{
		mBrokerIp = MDNS.IP(0);
		mBrokerPort = MDNS.port(0);
		Serial << endl
			   << "Found Broker Service" << mWifiSSID;
		Serial << " with IP Address [" << mBrokerIp;
		Serial << "] / Port [" << mBrokerPort << "]" << endl;
	}
	return brokerIp;
}

void NetworkManager::stateUpdate(const MqttClient *, const Topic &topic, const char *payload, size_t)
{
	Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl;
}

void NetworkManager::initClients()
{
	Serial << endl
		   << "--- Starting clients --- " << endl;

	Serial << "Connecting to broker with IP " << mBrokerIp << ":" << mBrokerPort << endl;
	mClientState.connect(mBrokerIp.toString().c_str(), mBrokerPort);
	mClientDataTransfer.connect(mBrokerIp.toString().c_str(), mBrokerPort);

	mClientState.setCallback(NetworkManager::stateUpdate);
	mClientState.subscribe(mTopicState);
}

void NetworkManager::loop()
{

	mBroker.loop();

	mClientState.loop();
	mClientDataTransfer.loop();

	static const int interval = 3000; // publishes every second
	static uint32_t timer = millis() + interval;

	if (millis() > timer)
	{
		Serial << "publishing " << mTopicState.c_str() << " from IP " << mIpAddressWifi.toString() << endl;
		timer += interval;
		mClientState.publish(mTopicState, "update " + mIpAddressWifi.toString());
	}
}
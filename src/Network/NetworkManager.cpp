#include "NetworkManager.h"
#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

NetworkManager::NetworkManager() :
	mIpAddressSubnet(255,255,255,0),
	mBroker(MQTT_PORT),
	mClientState(&mBroker),
	mClientDataTransfer(&mBroker),
	mAsyncWebServer(80)
	{	
}

void NetworkManager::begin(){
	
	setUniqueHostname();

	bool hasWifiConfig = loadWifiConfig();
	if (hasWifiConfig){
		initWifiSTA();
	} else {
		initWifiAP();
	}

	loadEthConfig();

	initMdns();
	startWebServer(hasWifiConfig);

}

void NetworkManager::setUniqueHostname(){
	char uniqueHostname[23];
	snprintf(uniqueHostname, 23, DEVICE_NAME"-%llX", ESP.getEfuseMac());

	Serial << "--- Setting unique hostname [" << uniqueHostname << "] ---" << endl;
    mUniqueHostname = uniqueHostname;
}

bool NetworkManager::loadWifiConfig(){
	Serial << "--- Loading WIFI Config---" << endl;
	
	String wifiSSID 	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_SSID);
    String wifiPassword	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_PWD);
	String wifiIP 		= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_IP);
	String wifiGateway 	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_WIFI_GATEWAY);
	
	if( wifiSSID == ""){
		Serial.println("Undefined SSID");
		return false;
	}
	mWifiSSID = wifiSSID;
	mWifiPassword = wifiPassword;
	
	if( wifiIP != "" || wifiGateway != ""){
		mHasCustomWifiAddress =
			mIpAddressWifi.fromString(wifiIP.c_str()) &&
			mIpAddressWifiGateway.fromString(wifiGateway.c_str());	
	}
	
	return true;
}

bool NetworkManager::loadEthConfig(){
	Serial << "--- Loading ETH Config---" << endl;

	String ethIP 		= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_ETH_IP);
	String ethGateway 	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + PARAM_ETH_GATEWAY);

	if( ethIP != "" || ethGateway != ""){
		mHasCustomEthAddress =
			mIpAddressEth.fromString(ethIP.c_str()) &&
			mIpAddressEthGateway.fromString(ethGateway.c_str());	
	}

	return true;
}

bool NetworkManager::initWifiAP(){
	Serial << "--- Initializing WIFI as Access Point ---" << endl;

	WiFi.softAP(mUniqueHostname.c_str(), NULL);

	Serial << "IP Address [" << WiFi.softAPIP();
  	Serial << "] / SSID [" << mUniqueHostname << "]" << endl;

	return true;
}

bool NetworkManager::initWifiSTA(){
	Serial << "--- Initializing WIFI as Station ---" << endl;

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(mUniqueHostname.c_str());

	if (mHasCustomWifiAddress && !WiFi.config(mIpAddressWifi, mIpAddressWifiGateway, mIpAddressSubnet)){
    	Serial.println("Failed to initialize with internal stored IPs");
    	return false;
  	}

	WiFi.begin(mWifiSSID.c_str(), mWifiPassword.c_str());
	
	Serial << "Connecting";
	unsigned long currentMillis = millis();
  	unsigned long previousMillis = currentMillis;

  	while (WiFi.status() != WL_CONNECTED) {
		currentMillis = millis();
		if (currentMillis - previousMillis >= TIMEOUT_NETWORK) {
			Serial.println("Failed to connect.");
			return false;
		}
		 Serial << '.'; 
		 delay(500); 
	}

  	Serial << endl << "Connected to " << mWifiSSID;
	Serial << " with IP Address [" << WiFi.localIP();
  	Serial << "] / MAC Address [" << WiFi.macAddress() << "]" << endl;

	return true;
}

bool NetworkManager::initETH(){
	Serial << "--- Initializing ETH ---" << endl;
	
	ETH.setHostname(mUniqueHostname.c_str());
	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

	if (mHasCustomEthAddress && !ETH.config(mIpAddressEth, mIpAddressEthGateway, mIpAddressSubnet)){
    	Serial.println("Failed to initialize with internal stored IPs");
    	return false;
  	}

	Serial << "Connecting";
	unsigned long currentMillis = millis();
  	unsigned long previousMillis = currentMillis;

  	while(!((uint32_t)ETH.localIP())) //wait for IP
	{
		currentMillis = millis();
		if (currentMillis - previousMillis >= TIMEOUT_NETWORK) {
			Serial.println("Failed to connect.");
			return false;
		}
		 Serial << '.'; 
		 delay(500); 
	}

	Serial << "IP Address [" << ETH.localIP();
  	Serial << "] / MAC Address [" << ETH.macAddress() << "]" << endl;

	return true;
}

void NetworkManager::initMdns(){
	Serial << "--- Starting mDNS " ;
	while( !MDNS.begin(DEVICE_NAME) ){ Serial << '.'; delay(1000); }
	Serial << " ---" << endl << "Service started" << endl;
}

void NetworkManager::startWebServer(bool hasWifiConfig){
	Serial << "--- Starting WebServer ---" << endl;

	mAsyncWebServer.on("/", HTTP_GET, std::bind(&NetworkManager::handleGetNetconfig, this, std::placeholders::_1));
	mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetworkManager::handlePostNetconfig, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/", SPIFFS, "/");
	
	if(hasWifiConfig) AsyncElegantOTA.begin(&mAsyncWebServer); // Start ElegantOTA
	    
    mAsyncWebServer.begin();
	MDNS.addService("http", "tcp", 80);
	Serial.println("Server started");
}

void NetworkManager::handleGetNetconfig(AsyncWebServerRequest *request){
	//request->send(SPIFFS, "/netconfig.html", "text/html");
	request->send(SPIFFS, "/netconfig.html", String(), false, [this](const String &var) -> String { return this->processTemplateNetconfig(var); });
}

String NetworkManager::processTemplateNetconfig(const String& var)
{
	if(var == PARAM_WIFI_SSID)
    	return F(mWifiSSID.c_str());
	if(var == PARAM_WIFI_PWD)
    	return F(mWifiPassword.c_str());

	String ipAddress;
	if(var == PARAM_WIFI_IP){
		ipAddress = mIpAddressWifi.toString();
		if (ipAddress != "0.0.0.0") return F(mIpAddressWifi.toString().c_str());
	}
	if(var == PARAM_WIFI_GATEWAY){
		ipAddress = mIpAddressWifiGateway.toString();
		if (ipAddress != "0.0.0.0") return F(mIpAddressWifiGateway.toString().c_str());
	}
	if(var == PARAM_ETH_IP){
		ipAddress = mIpAddressEth.toString();
		if (ipAddress != "0.0.0.0") return F(mIpAddressEth.toString().c_str());
	}
	if(var == PARAM_ETH_GATEWAY){
		ipAddress = mIpAddressEthGateway.toString();
		if (ipAddress != "0.0.0.0") return F(mIpAddressEthGateway.toString().c_str());
	}
		

	return String();
}

void NetworkManager::handlePostNetconfig(AsyncWebServerRequest *request){
	
	int params = request->params();
	for(int i=0;i<params;i++){
		AsyncWebParameter* p = request->getParam(i);
		if(p->isPost()){
			writeParameterToSPIFFS(p, PARAM_WIFI_SSID);
			writeParameterToSPIFFS(p, PARAM_WIFI_PWD);
			writeParameterToSPIFFS(p, PARAM_WIFI_IP);
			writeParameterToSPIFFS(p, PARAM_WIFI_GATEWAY);
			writeParameterToSPIFFS(p, PARAM_ETH_IP);
			writeParameterToSPIFFS(p, PARAM_ETH_GATEWAY);
		}
	}
	request->send(200, "text/plain", "Updated ESP settings. Rebooting now.");
	delay(3000);
	ESP.restart();
}

void NetworkManager::writeParameterToSPIFFS(AsyncWebParameter* p, String parameter){
	if (p->name() == parameter) {
		mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + parameter, p->value().c_str());
	}
}


void NetworkManager::startBroker(){
	Serial << "--- Starting local broker --- " << endl;
	mBroker.begin();
	MDNS.addService("mqtt", "tcp", MQTT_PORT);
}

IPAddress NetworkManager::findStatusBroker(){
	//while(mdns_init()!= ESP_OK){ Serial << '.'; delay(1000); }
	IPAddress brokerIp;
	Serial << " --- Resolving Broker IP";
	while (brokerIp.toString() == "0.0.0.0") {
		Serial << '.';
		brokerIp = MDNS.queryHost(DEVICE_NAME);
		delay(1000);
	}
	int nrOfServices = MDNS.queryService("mqtt", "tcp");
	if (nrOfServices == 0) Serial.println("No services were found.");
	else {
		mBrokerIp = MDNS.IP(0);
		mBrokerPort = MDNS.port(0);
		Serial << endl << "Found Broker Service" << mWifiSSID;
		Serial << " with IP Address [" << mBrokerIp;
  		Serial << "] / Port [" << mBrokerPort << "]" << endl;
	}
	return brokerIp;
}

void NetworkManager::stateUpdate(const MqttClient*, const Topic& topic, const char* payload, size_t ){ 
	Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl; 
}

void NetworkManager::initClients(){
	Serial << "--- Starting clients --- " << endl;

	Serial << "Connecting to broker with IP " << mBrokerIp << ":" << mBrokerPort << endl;
	mClientState.connect(mBrokerIp.toString().c_str(), mBrokerPort);
	mClientDataTransfer.connect(mBrokerIp.toString().c_str(), mBrokerPort);

	mClientState.setCallback(NetworkManager::stateUpdate);
	mClientState.subscribe(mTopicState);

}

void NetworkManager::loop(){
	
	mBroker.loop();

	mClientState.loop();
	mClientDataTransfer.loop();

	static const int interval = 3000;  // publishes every second
    static uint32_t timer = millis() + interval;

    if (millis() > timer)
    {
      Serial << "publishing " << mTopicState.c_str() << " from IP " << mIpAddressWifi.toString() << endl;
      timer += interval;
      mClientState.publish(mTopicState, "update " + mIpAddressWifi.toString());
    }
  
}
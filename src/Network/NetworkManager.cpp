#include "NetworkManager.h"
#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

NetworkManager::NetworkManager() :
	mIpAddressWifiSubnet(255, 255, 0, 0),
	mBroker(BROKER_PORT),
	mClientState(&mBroker),
	mClientDataTransfer(&mBroker),
	mAsyncWebServer(80)
	{	
}

bool NetworkManager::loadWifiConfig(){
	Serial << "--- Loading WIFI Config---" << endl;
	
	String wifiSSID 	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + mPersistenceManager.PARAM_WIFI_SSID);
    String wifiPassword	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + mPersistenceManager.PARAM_WIFI_PWD);
	String wifiIP 		= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + mPersistenceManager.PARAM_WIFI_IP);
	String wifiGateway 	= mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + mPersistenceManager.PARAM_WIFI_GATEWAY);
		
	if( wifiSSID == "" || wifiIP == ""){
		Serial.println("Undefined SSID or IP address.");
		return false;
	}
	mWifiSSID = wifiSSID;
	mWifiPassword = wifiPassword;
	mIpAddressWifi.fromString(wifiIP.c_str());
	mIpAddressWifiGateway.fromString(wifiGateway.c_str());	
	return true;
}

bool NetworkManager::loadEthConfig(){
	Serial << "--- Loading ETH Config---" << endl;

	String ethIP = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "" + mPersistenceManager.PARAM_ETH_IP);

	if(ethIP == ""){
		Serial.println("Undefined IP address.");
		return false;
	}
	mIpAddressEth.fromString(ethIP.c_str());
	return true;
}

bool NetworkManager::initWifiAP(){
	Serial << "--- Initializing WIFI as Access Point ---" << endl;
	char APssid[25];
	uint64_t macAddress = ESP.getEfuseMac();
 	uint64_t macAddressTrunc = macAddress << 40;
 	uint64_t chipID = macAddressTrunc >> 40;
  	snprintf(APssid,25, "ESP-WIFI-MANAGER-%08X",chipID);
    WiFi.softAP(APssid, NULL);

    IPAddress IP = WiFi.softAPIP();
	Serial << "AP SSID: " << APssid << endl;
	Serial << "AP IP address: " << IP << endl;
	
	return true;
}

bool NetworkManager::initWifiSTA(){
	Serial << "--- Initializing WIFI as Station ---" << endl;

	WiFi.mode(WIFI_STA);
	//WiFi.setHostname("YOUR_NEW_HOSTNAME");

	if (!WiFi.config(mIpAddressWifi, mIpAddressWifiGateway, mIpAddressWifiSubnet)){
    	Serial.println("STA Failed to configure");
    	return false;
  	}

	WiFi.begin(mWifiSSID.c_str(), mWifiPassword.c_str());
	Serial << "Connecting";
	
	unsigned long previousMillis = 0;
	const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
	unsigned long currentMillis = millis();
  	previousMillis = currentMillis;
  	while (WiFi.status() != WL_CONNECTED) {
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			Serial.println("Failed to connect.");
			return false;
		}
		 Serial << '.'; 
		 delay(500); 
	}

  	Serial << endl << "Connected to " << mWifiSSID;
	Serial << " with IP Address [" << WiFi.localIP();
  	Serial << "] / MAC Address [" << WiFi.macAddress() << "]" << endl;

	if (mMacAddressWifi == String(BROKER_MAC)) this->isMaster = true;

	return true;
}

void NetworkManager::initETH(){
	Serial << "--- Connecting via ETH " << endl;

	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
	int count = 2;
	int *p_cnt = &count;
	IPAddress ethIp(192, 168, 0, *p_cnt);
	IPAddress gateway(192, 168, 0, 1);
	IPAddress subnet(255, 255, 255, 0);
	Serial << "trying IP " << ethIp.toString() << endl;
  	while(!ETH.config(ethIp, gateway, subnet)){
		count++;
		Serial << "trying IP " << ethIp.toString() << endl;
	}

	while(!((uint32_t)ETH.localIP())) //wait for IP
  	{

  	}
	Serial << "IP Address: " << ETH.localIP() << endl;
  	Serial << "MAC Address: " << ETH.macAddress() << endl;
}

String NetworkManager::processConfigTemplate(const String& var)
{
	if(var == "WIFI_SSID")
    	return F(mWifiSSID.c_str());
	if(var == "WIFI_IP")
		return F(mIpAddressWifi.toString().c_str());
	if(var == "WIFI_GATEWAY")
		return F(mIpAddressWifiGateway.toString().c_str());
	if(var == "ETH_IP")
		return F(mIpAddressEth.toString().c_str());
	return String();
}

String test(const String& var)
{
	if(var == "WIFI_SSID")
    	return F("mWifiSSID");
	if(var == "WIFI_IP")
		return F("mIpAddressWifi.toString().c_str()");
	if(var == "WIFI_GATEWAY")
		return F("mIpAddressWifiGateway.toString().c_str()");
	if(var == "ETH_IP")
		return F("mIpAddressEth.toString().c_str()");
	return String();
}

void NetworkManager::handleConfigGetAP(AsyncWebServerRequest *request){
	request->send(SPIFFS, "/config_AP.html", "text/html");
}

void NetworkManager::handleConfigGetSTA(AsyncWebServerRequest *request){
	request->send(SPIFFS, "/config_STA.html", String(), false, test);
	//request->send(SPIFFS, "/config_STA.html", "text/html");
}

void NetworkManager::writeParameterToSPIFFS(AsyncWebParameter* p, String parameter){
	if (p->name() == parameter) {
		mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + parameter, p->value().c_str());
	}
}

void NetworkManager::handleConfigPost(AsyncWebServerRequest *request){
	String newWifiIp = "";

	int params = request->params();
		for(int i=0;i<params;i++){
			AsyncWebParameter* p = request->getParam(i);
			if(p->isPost() && p->value() != ""){
				if (p->name() == mPersistenceManager.PARAM_WIFI_IP) newWifiIp = p->value();
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_SSID);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_PWD);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_IP);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_GATEWAY);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_ETH_IP);
			}
		}
      request->send(200, "text/plain", "Updated ESP settings. Rebooting now. You can access the config through your local wifi with ip " + newWifiIp);
      delay(3000);
      ESP.restart();
}

void NetworkManager::startWebServer(bool APmode){
	Serial << "--- Starting WebServer ";

	if(APmode){
		Serial << "in AP mode ---" << endl;
		mAsyncWebServer.on("/", HTTP_GET, std::bind(&NetworkManager::handleConfigGetAP, this, std::placeholders::_1));
    	mAsyncWebServer.serveStatic("/", SPIFFS, "/");
    	mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetworkManager::handleConfigPost, this, std::placeholders::_1));
	}
	else {
		Serial << "in Station mode ---" << endl;
		mAsyncWebServer.on("/", HTTP_GET, std::bind(&NetworkManager::handleConfigGetSTA, this, std::placeholders::_1));
    	mAsyncWebServer.serveStatic("/", SPIFFS, "/");
		mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetworkManager::handleConfigPost, this, std::placeholders::_1));
		AsyncElegantOTA.begin(&mAsyncWebServer); // Start ElegantOTA
	}
	    
    mAsyncWebServer.begin();
	Serial.println("Server started");
}

void NetworkManager::startBroker(){
	Serial << "--- Starting local broker --- " << endl;
	mBroker.begin();	
}

void NetworkManager::initMdns(){
	Serial << "--- Starting mDNS " ;
	if (isMaster){
		while(!MDNS.begin(BROKER_HOSTNAME)){ Serial << '.'; delay(1000); }
		MDNS.addService("mqtt", "tcp", BROKER_PORT);
		Serial << " ---" << endl << "Responder started and mqtt service added" << endl;
	}
	else {
		while(mdns_init()!= ESP_OK){ Serial << '.'; delay(1000); }

		IPAddress brokerIp;
		Serial << " ---" << endl << "Resolving Broker IP";
		while (brokerIp.toString() == "0.0.0.0") {
			Serial << '.';
			brokerIp = MDNS.queryHost(BROKER_HOSTNAME);
			delay(1000);
		}
		int nrOfServices = MDNS.queryService("mqtt", "tcp");
		if (nrOfServices == 0) Serial.println("No services were found.");
		else {
			mBrokerIp = MDNS.IP(0);
			mBrokerPort = MDNS.port(0);
			Serial << endl <<  "Found broker service" << endl;
			Serial << "IP Address: " << mBrokerIp << endl;
			Serial << "Port: " << mBrokerPort << endl;
		}
	}
}

void NetworkManager::stateUpdate(const MqttClient*, const Topic& topic, const char* payload, size_t ){ 
	Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl; 
}

void NetworkManager::initClients(){
	Serial << "--- Starting clients --- " << endl;
	if(isMaster){
		Serial << "Already connected to local broker" << endl;
	}
	else {
		Serial << "Connecting to broker with IP " << mBrokerIp << ":" << mBrokerPort << endl;
		mClientState.connect(mBrokerIp.toString().c_str(), mBrokerPort);
		mClientDataTransfer.connect(mBrokerIp.toString().c_str(), mBrokerPort);
	}

	mClientState.setCallback(NetworkManager::stateUpdate);
	mClientState.subscribe(mTopicState);

}

void NetworkManager::loop(){
	
	if(isMaster) mBroker.loop();

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
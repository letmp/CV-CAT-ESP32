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

void NetworkManager::writeParameterToSPIFFS(AsyncWebParameter* p, String parameter){
	if (p->name() == parameter) {
		mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + parameter, p->value().c_str());
	}
}

void NetworkManager::writeWifiParametersToSPIFFS(AsyncWebServerRequest *request){
	int params = request->params();
		for(int i=0;i<params;i++){
			AsyncWebParameter* p = request->getParam(i);
			if(p->isPost()){
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_SSID);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_PWD);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_IP);
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_WIFI_GATEWAY);
			}
		}
      request->send(200, "text/plain", "Updated WIFI Settings. ESP will restart now with updated settings");
      delay(3000);
      ESP.restart();
}

void NetworkManager::writeEthParametersToSPIFFS(AsyncWebServerRequest *request){
	int params = request->params();
		for(int i=0;i<params;i++){
			AsyncWebParameter* p = request->getParam(i);
			if(p->isPost()){
				writeParameterToSPIFFS(p, mPersistenceManager.PARAM_ETH_IP);
			}
		}
      request->send(200, "text/plain", "Updated ETH Settings. ESP will restart now with updated settings.");
      delay(3000);
      ESP.restart();
}

void NetworkManager::stateUpdate(const MqttClient*, const Topic& topic, const char* payload, size_t ){ 
	Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl; 
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
	// NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 
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

void NetworkManager::startWebServerAP(){
	Serial << "--- Starting WebServer in Access Point Mode--- " << endl;

    mAsyncWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    mAsyncWebServer.serveStatic("/", SPIFFS, "/");
    mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetworkManager::writeWifiParametersToSPIFFS, this, std::placeholders::_1));
    mAsyncWebServer.begin();
}

void NetworkManager::startWebServerSTA(){
	Serial << "--- Starting WebServer in Station Mode--- " << endl;

	mAsyncWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
    });
    mAsyncWebServer.serveStatic("/", SPIFFS, "/");

	AsyncElegantOTA.begin(&mAsyncWebServer); // Start ElegantOTA

	mAsyncWebServer.begin();
	Serial.println("HTTP server started");
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
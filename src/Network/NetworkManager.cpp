#include "NetworkManager.h"

#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

NetworkManager::NetworkManager() :
	 _broker(BROKER_PORT),
	 _clientState(&_broker),
	 _clientDataTransfer(&_broker),
	 _server(80)
	 {	
}

void NetworkManager::initWifi(){
	Serial << "--- Connecting via WIFI ";

	if (strlen(WIFI_SSID)==0)
		Serial << "****** MISSING SSID! RENAME Credentials-Default.h to Credentials.h and add SSID/PASSWORD *************" << endl;
	
	WiFi.mode(WIFI_STA);
	//WiFi.setHostname("YOUR_NEW_HOSTNAME");
	WiFi.begin(WIFI_SSID, WIFI_PWD);
  	while (WiFi.status() != WL_CONNECTED) { Serial << '.'; delay(500); }

  	Serial << " ---" << endl << "Connected to " << WIFI_SSID << endl;
	Serial << "IP Address: " << WiFi.localIP() << endl;
  	Serial << "MAC Address: " << WiFi.macAddress() << endl;

	_ipAddress = WiFi.localIP();
	_macAddress = WiFi.macAddress();

	if (_macAddress == String(BROKER_MAC)) this->isMaster = true;
}

#define ETH_ADDR        1
#define ETH_POWER_PIN   5
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define ETH_TYPE        ETH_PHY_IP101

void NetworkManager::initETH(){
	Serial << "--- Connecting via ETH ";

	ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE);
	int count = 2;
	int *p_cnt = &count;
	IPAddress ethIp(192, 168, 0, *p_cnt);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);
	Serial << "trying IP " << ethIp.toString() << endl;
  	while(!ETH.config(ethIp, gateway, subnet)){
		count++;
		Serial << "trying IP " << ethIp.toString() << endl;
	}

	Serial << "IP Address: " << ETH.localIP() << endl;
  	Serial << "MAC Address: " << ETH.macAddress() << endl;
	_ipAddress = ETH.localIP();
	_macAddress = ETH.macAddress();

	if (_macAddress == String(BROKER_MAC)) this->isMaster = true;
}

void NetworkManager::startWebServer(){
	Serial << "--- Starting OTA WebServer --- " << endl;
	_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    	request->send(200, "text/plain", "Hi! I am ESP32.");
  	});

	AsyncElegantOTA.begin(&_server);    // Start ElegantOTA
	_server.begin();
	Serial.println("HTTP server started");
}

void NetworkManager::startBroker(){
	Serial << "--- Starting local broker --- " << endl;
	_broker.begin();	
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
			_brokerIp = MDNS.IP(0);
			_brokerPort = MDNS.port(0);
			Serial << endl <<  "Found broker service" << endl;
			Serial << "IP Address: " << _brokerIp << endl;
			Serial << "Port: " << _brokerPort << endl;
		}
	}
}

void NetworkManager::initClients(){
	Serial << "--- Starting clients --- " << endl;
	if(isMaster){
		Serial << "Already connected to local broker" << endl;
	}
	else {
		Serial << "Connecting to broker with IP " << _brokerIp << ":" << _brokerPort << endl;
		_clientState.connect(_brokerIp.toString().c_str(), _brokerPort);
		_clientDataTransfer.connect(_brokerIp.toString().c_str(), _brokerPort);
	}

	_clientState.setCallback(NetworkManager::stateUpdate);
	_clientState.subscribe(_topicState);

}

void NetworkManager::stateUpdate(const MqttClient*, const Topic& topic, const char* payload, size_t ){ 
	Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl; 
}

void NetworkManager::loop(){
	
	if(isMaster) _broker.loop();

	_clientState.loop();
	_clientDataTransfer.loop();

	static const int interval = 3000;  // publishes every second
    static uint32_t timer = millis() + interval;

    if (millis() > timer)
    {
      Serial << "publishing " << _topicState.c_str() << " from IP " << _ipAddress.toString() << endl;
      timer += interval;
      _clientState.publish(_topicState, "update " + _ipAddress.toString());
    }
  
}
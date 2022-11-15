#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <ESPmDNS.h>
#include <ETH.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Credentials/Credentials.h>
#include <Common/Globals.h>

class NetworkManager {
	
	private:
		IPAddress _ipAddress;
		String _macAddress;
		
		AsyncWebServer _server;

		MqttBroker _broker;
		IPAddress _brokerIp;
		uint16_t _brokerPort;
		
		MqttClient _clientState;
		MqttClient _clientDataTransfer;
		std::string _topicState="states/update";

		static void stateUpdate(const MqttClient*, const Topic& topic, const char* payload, size_t );

	public:
		bool isMaster = false;
		
		NetworkManager();
		
		void initWifi();
		void initETH();

		void startWebServer();

		void startBroker();
		void initMdns();
		void initClients();
		
		void loop();
};

#endif
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <ESPmDNS.h>

#include <Credentials/Credentials.h>
#include <Globals.h>

class NetworkManager {
	
	private:
		IPAddress _ipAddress;
		String _macAddress;
		bool _localBroker = false;

		MqttBroker _broker(int);
		IPAddress _brokerIp;
		uint16_t _brokerPort;
		
		MqttClient _clientState;
		MqttClient _clientDataTransfer;

	public:
		
		NetworkManager();
		void initWifi();
		void initBroker();
		void initMdns();
		void initClients();

		void loop();
};

#endif
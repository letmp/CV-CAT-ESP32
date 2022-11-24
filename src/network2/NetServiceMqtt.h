#ifndef NET_SERVICE_MQTT_H
#define NET_SERVICE_MQTT_H

#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt

#include <Hardware/HardwareManager.h>

#include <Network/NetConstants.h>
#include <Network/NetConfig.h>

class NetServiceMqtt
{

private:
	
    HardwareManager &rHardwareManager;
	NetConfig &rNetConfig;
	
	MqttBroker mBroker;
	IPAddress mBrokerIp;
	uint16_t mBrokerPort;
	MqttClient mClientState;
	MqttClient mClientDataTransfer;
	std::string mTopicState = "states/update";

	static void stateUpdate(const MqttClient *, const Topic &topic, const char *payload, size_t);

public:
	NetServiceMqtt(NetConfig& networkData, HardwareManager& hardwareManager);
	void begin();
	
	void initClients();
	IPAddress findStatusBroker();
	
	void loop();
};

#endif
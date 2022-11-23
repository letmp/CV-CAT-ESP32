#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt

#include <Hardware/HardwareManager.h>

#include <Network/ConstantProvider.h>
#include <Network/NetworkData.h>

class MqttManager
{

private:
	
    HardwareManager &mHardwareManager;

	NetworkData &mNetworkData;
	
	MqttBroker mBroker;
	IPAddress mBrokerIp;
	uint16_t mBrokerPort;
	MqttClient mClientState;
	MqttClient mClientDataTransfer;
	std::string mTopicState = "states/update";

	static void stateUpdate(const MqttClient *, const Topic &topic, const char *payload, size_t);

public:
	MqttManager(NetworkData& networkData, HardwareManager& hardwareManager);
	void begin();
	
	void initClients();
	IPAddress findStatusBroker();
	
	void loop();
};

#endif
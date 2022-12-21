#ifndef NET_SERVICE_MQTT_H
#define NET_SERVICE_MQTT_H

#include <Arduino.h>
#include <TinyMqtt.h> // https://github.com/hsaturn/TinyMqtt
#include <ESPmDNS.h>
#include <Streaming.h>
#include <map>

#include <hardware/HardwareManager.h>
#include <network/NetConstants.h>
#include <network/NetConfig.h>


class NetServiceMqtt
{

private:
	HardwareManager &rHardwareManager;
	NetConfig &rNetConfig;

	MqttBroker mBroker;

	MqttClient mClientLocal;
	std::map<String, MqttClient*> mClientMap;
	
	std::string mTopicState = "states/update";
	std::string mTopicData = "data/update";

	void addRemoteBroker(const String &ip);

	void subscribeClient(MqttClient *client);
	
	void notifyBroker(MqttClient client, std::string topic, String message);
	static void callbackFunction(const MqttClient *, const Topic &topic, const char *payload, size_t);

public:
	NetServiceMqtt(NetConfig &networkData, HardwareManager &hardwareManager);
	void begin();

	void findRemoteBrokers();
	void updateRemoteBrokerList();

	// update remotebrokerlist on notify
	// listen to local broker topics
	// send to local/remote broker topics

	void loop();
};

#endif
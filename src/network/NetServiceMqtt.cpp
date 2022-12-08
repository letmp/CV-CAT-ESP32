#include "NetServiceMqtt.h"

NetServiceMqtt::NetServiceMqtt(NetConfig &nd, HardwareManager &hm) : rHardwareManager{hm},
                                                                     rNetConfig{nd},
                                                                     mBroker(NetConstants::PORT_MQTT),
                                                                     mClientLocal(&mBroker)
{
}

void NetServiceMqtt::begin()
{
    Serial << endl << "--- Starting local MQTT broker --- " << endl;
    mBroker.begin();

    initClient(mClientLocal);
    
    mClientMap.emplace(rNetConfig.ethIp.toString(), mClientLocal);
    
    Serial << "Elements: [";
    for(auto it = mClientMap.cbegin(); it != mClientMap.cend(); ++it){
    Serial << it->first << ",";
    }
    Serial << "]" << endl;
}

void NetServiceMqtt::findRemoteBrokers()
{
    Serial << endl << "--- Searching Remote Broker ---" << endl;

    int nrOfServices = MDNS.queryService("mqtt", "tcp");
    for (int i = 0; i < nrOfServices; i = i + 1)
    {
        String ethIpStr = MDNS.txt(i, "eth");
        String wifiIpStr = MDNS.txt(i, "wifi");
        Serial << "Found [" << MDNS.hostname(i) << "] / ";
        Serial << "IP Address [" << MDNS.IP(i) << ":" << MDNS.port(i) << "] / ";
        Serial << "ETH Value [" << ethIpStr << "] / ";
        Serial << "Wifi Value [" << wifiIpStr << "] " << endl;
        if (ethIpStr != "0.0.0.0")
        {
            if (mClientMap.find(ethIpStr) == mClientMap.end()){
                insertClient(ethIpStr, true);
            }
        }
        /*else if (wifiIpStr != "0.0.0.0")
        {
            if (mClientMap.find(wifiIpStr) == mClientMap.end()){
                insertClient(wifiIpStr, false);
            }
        }*/
        Serial << endl;
    }
}

void NetServiceMqtt::insertClient(String ip, bool isEth)
{
    MqttClient clientRemote;
    mClientMap.emplace(ip, clientRemote);

    clientRemote.connect(ip.c_str(), NetConstants::PORT_MQTT);
    clientRemote.setCallback(NetServiceMqtt::callbackFunction);
    clientRemote.subscribe(mTopicState);

    

    //initClient(clientRemote);

    //mClientMap.emplace(ip, clientRemote);
    //mClientMap.emplace(std::make_pair(ip, clientRemote));
    
    if (isEth)
    {
        //clientRemote.publish(mTopicState, "Connected via ETH " + rNetConfig.ethIp.toString());
        Serial << "--> Added ETH Broker [" << ip.c_str() << ":" << NetConstants::PORT_MQTT << "]" << endl;
    }
    else
    {
        //clientRemote.publish(mTopicState, "Connected via WIFI " + rNetConfig.wifiIp.toString());
        Serial << "--> Added Wifi Broker" << ip.c_str() << ":" << NetConstants::PORT_MQTT << "]" << endl;
    }

    Serial << "Elements: [";
    for(auto it = mClientMap.cbegin(); it != mClientMap.cend(); ++it){
    Serial << it->first << ",";
    }
    Serial << "]" << endl;
}

void NetServiceMqtt::initClient(MqttClient client)
{
    client.setCallback(NetServiceMqtt::callbackFunction);
    client.subscribe(mTopicState);
    client.subscribe(mTopicData);
}

void NetServiceMqtt::notifyBroker(MqttClient client, std::string topic, String message)
{
    client.publish(topic, message);
}

void NetServiceMqtt::callbackFunction(const MqttClient *, const Topic &topic, const char *payload, size_t)
{
    Serial << "--> Callback [" << topic.c_str() << "] [" << payload << "]" << endl;
}

void NetServiceMqtt::loop()
{
}
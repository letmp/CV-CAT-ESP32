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

    mClientMap[rNetConfig.ethIp.toString()] = &mClientLocal;
    mClientLocal.subscribe(mTopicState);
    mClientLocal.subscribe(mTopicData);
    mClientLocal.setCallback(NetServiceMqtt::callbackFunction);

    Serial << "Added local broker @ IP [" << rNetConfig.ethIp.toString() << "]" << endl;
    
}

void NetServiceMqtt::findRemoteBrokers()
{
    Serial << endl << "--- Searching remote MQTT brokers ---" << endl;

    int nrOfServices = MDNS.queryService("mqtt", "tcp");
    for (int i = 0; i < nrOfServices; i = i + 1)
    {
        String ethIpStr = MDNS.txt(i, "eth");
        String wifiIpStr = MDNS.txt(i, "wifi");
        Serial << "Found remote broker [" << MDNS.hostname(i) << "] @ ";
        Serial << "IP [" << MDNS.IP(i) << ":" << MDNS.port(i) << "] / ";
        /*if (ethIpStr != "0.0.0.0")
        {
            if (mClientMap.find(ethIpStr) == mClientMap.end())
            {
                addRemoteBroker(ethIpStr);
                Serial << "--> added broker via ETH Address [" << ethIpStr << "]";
            }
            else
                Serial << "--> ETH address already stored";
        }
        else */if (wifiIpStr != "0.0.0.0")
        {
            if (mClientMap.find(wifiIpStr) == mClientMap.end())
            {
                addRemoteBroker(wifiIpStr);
                Serial << "--> added broker via WIFI Address [" << wifiIpStr << "]";
            }
            else
                Serial << "--> WIFI address already stored";
        }
        Serial << endl;
    }
}

void NetServiceMqtt::addRemoteBroker(const String &ip)
{
    /*MqttClient* client = new MqttClient();
    
    client->subscribe(mTopicState);
    client->subscribe(mTopicData);
    client->setCallback(NetServiceMqtt::callbackFunction);
    client->connect(ip.c_str(), NetConstants::PORT_MQTT);
    mClientMap[ip] = client;
    */
    mClientRemote.connect(ip.c_str(), NetConstants::PORT_MQTT);
    mClientRemote.subscribe(mTopicState);
    mClientRemote.subscribe(mTopicData);
    mClientRemote.setCallback(NetServiceMqtt::callbackFunction);
    mClientMap[ip] = &mClientRemote;
    
}

void NetServiceMqtt::subscribeClient(MqttClient &client)
{
    client.subscribe(mTopicState);
    client.subscribe(mTopicData);
    client.setCallback(NetServiceMqtt::callbackFunction);
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
    mBroker.loop();
    mClientLocal.loop();
    mClientRemote.loop();
    mClientRemote.publish(mTopicState, String("ping from " + rNetConfig.ethIp.toString()) );

    /*for (std::map<String, MqttClient*>::iterator iter = mClientMap.begin(); iter != mClientMap.end(); ++iter)
    {
        iter->second->loop();
        if(iter->first != rNetConfig.ethIp.toString())
            iter->second->publish(mTopicState, String("ping from " + rNetConfig.ethIp.toString() + " to" + iter->first));
    }*/
    
}
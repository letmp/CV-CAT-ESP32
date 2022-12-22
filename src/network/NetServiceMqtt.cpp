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

    subscribeClient(&mClientLocal);
    mClientMap[rNetConfig.ethIp.toString()] = &mClientLocal;

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
        if (ethIpStr != "0.0.0.0")
        {
            if (mClientMap.find(ethIpStr) == mClientMap.end())
            {
                Serial << "--> adding broker via ETH Address [" << ethIpStr << "]";
                addRemoteBroker(ethIpStr);
            }
            else
                Serial << "--> ETH address already stored";
        }
        else if (wifiIpStr != "0.0.0.0")
        {
            if (mClientMap.find(wifiIpStr) == mClientMap.end())
            {
                Serial << "--> adding broker via WIFI Address [" << wifiIpStr << "] ";
                addRemoteBroker(wifiIpStr);
            }
            else
                Serial << "--> WIFI address already stored";
        }
        Serial << endl;
    }
}

void NetServiceMqtt::addRemoteBroker(const String &ip)
{
    MqttClient* pNewClient = new MqttClient();
    unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis;

    while(not pNewClient->connected()) {
        currentMillis = millis();
        pNewClient->connect(ip.c_str(), NetConstants::PORT_MQTT, 86400); // keep alive 86400 = 24hours
        if (currentMillis - previousMillis >= NetConstants::CON_TIMEOUT)
		{
			Serial <<" failed!";
			break;
		}
        Serial << ".";
        delay(500);
    }
    
    subscribeClient(pNewClient);

    mClientMap[ip] = pNewClient;
    
}

void NetServiceMqtt::subscribeClient(MqttClient *client)
{
    client->subscribe(mTopicState);
    client->subscribe(mTopicData);
    client->setCallback(NetServiceMqtt::callbackFunction);
    //client->setCallback(std::bind(&NetServiceMqtt::callBackBind, this, std::placeholders::_1));
}

void NetServiceMqtt::loop()
{
    mBroker.loop();
    
    static const int intervalA = 5000;  // publishes every 5s (please avoid usage of delay())
    static uint32_t timerA = millis() + intervalA;

    if (millis() > timerA)
    {
        timerA += intervalA;

        for (std::map<String, MqttClient*>::iterator iter = mClientMap.begin(); iter != mClientMap.end(); ++iter)
        {
            iter->second->loop();

            if (not iter->second->connected()){
                Serial << millis() << " // not connected to ip [" <<iter->first << "] -> removing from clientlist" << endl;
                mClientMap.erase(iter);
            }
            else
                iter->second->publish(mTopicState, String("ping from " + rNetConfig.ethIp.toString() + " to " + iter->first));
        }
    }    
}

void NetServiceMqtt::callbackFunction(const MqttClient *, const Topic &topic, const char *payload, size_t)
{
    
    Serial << "--> Callback [" << topic.c_str() << "] [" << payload << "]" << endl;
}

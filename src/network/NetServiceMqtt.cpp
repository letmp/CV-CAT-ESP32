#include "NetServiceMqtt.h"

NetServiceMqtt::NetServiceMqtt(NetConfig &nd, HardwareManager &hm) : rHardwareManager{hm},
                                                               rNetConfig{nd},
                                                               mBroker(NetConstants::PORT_MQTT),
                                                               mClientState(&mBroker),
                                                               mClientDataTransfer(&mBroker)
{
}

void NetServiceMqtt::begin()
{
    Serial << endl << "--- Starting local broker --- " << endl;
    mBroker.begin();
}

IPAddress NetServiceMqtt::findStatusBroker()
{
    // while(mdns_init()!= ESP_OK){ Serial << '.'; delay(1000); }
    IPAddress brokerIp;
    /*
    Serial << endl << " --- Resolving Broker IP";
     while (brokerIp.toString() == "0.0.0.0")
     {
         Serial << '.';
         brokerIp = MDNS.queryHost(DEVICE_NAME);
         delay(1000);
     }
     int nrOfServices = MDNS.queryService("mqtt", "tcp");
     if (nrOfServices == 0)
         Serial.println("No services were found.");
     else
     {
         mBrokerIp = MDNS.IP(0);
         mBrokerPort = MDNS.port(0);
         Serial << endl
                << "Found Broker Service" << mWifiSSID;
         Serial << " with IP Address [" << mBrokerIp;
         Serial << "] / Port [" << mBrokerPort << "]" << endl;
     }
     */
    return brokerIp;
}

void NetServiceMqtt::initClients()
{
    Serial << endl
           << "--- Starting clients --- " << endl;

    Serial << "Connecting to broker with IP " << mBrokerIp << ":" << mBrokerPort << endl;
    mClientState.connect(mBrokerIp.toString().c_str(), mBrokerPort);
    mClientDataTransfer.connect(mBrokerIp.toString().c_str(), mBrokerPort);

    mClientState.setCallback(NetServiceMqtt::stateUpdate);
    mClientState.subscribe(mTopicState);
}

void NetServiceMqtt::stateUpdate(const MqttClient *, const Topic &topic, const char *payload, size_t)
{
    Serial << "--> stateUpdate received " << topic.c_str() << ", " << payload << endl;
}

void NetServiceMqtt::loop()
{
}
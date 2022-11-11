#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <ESPmDNS.h>

#include <Credentials/Credentials.h>
#include <Globals.h>

#include <Network/NetworkManager.h>

NetworkManager networkManager;


void setup()
{
  Serial.begin(115200);
  delay(500);
  
  networkManager.initWifi();
  if(networkManager.isMaster) networkManager.startBroker();

  networkManager.initMdns();
  networkManager.initClients();
  
}

void loop()
{
  networkManager.loop();
}

#include <Arduino.h>
#include <Network/NetworkManager.h>

NetworkManager networkManager;

void setup()
{
  Serial.begin(115200);
  delay(500);
  
  networkManager.initWifi();
  //networkManager.initETH();
  networkManager.startWebServer();
  if(networkManager.isMaster) networkManager.startBroker();

  networkManager.initMdns();
  networkManager.initClients();
  
}

void loop()
{
  networkManager.loop();
}

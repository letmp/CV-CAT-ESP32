#include <Arduino.h>
#include <Network/NetworkManager.h>

NetworkManager networkManager;

void setup()
{
  Serial.begin(115200);
  delay(5000);

  networkManager.begin();

  // networkManager.initETH();
  // if(networkManager.isMaster) networkManager.startBroker();
  // networkManager.initMdns();
  // networkManager.initClients();
}

void loop()
{
  // networkManager.loop();
}

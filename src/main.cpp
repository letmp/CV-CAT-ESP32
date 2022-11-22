#include <Arduino.h>
#include <Network/NetworkManager.h>
#include <IO/IOManager.h>

NetworkManager networkManager;
IOManager ioManager;

void setup()
{
  Serial.begin(115200);
  delay(5000);

  networkManager.begin();
  
  ioManager.begin();


  // if(networkManager.isMaster) networkManager.startBroker();
  // networkManager.initMdns();
  // networkManager.initClients();
}

void loop()
{
  ioManager.loop();
  // networkManager.loop();
}

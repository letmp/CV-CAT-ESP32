#include <Arduino.h>
#include <Network/NetworkManager.h>

NetworkManager networkManager;

void setup()
{
  Serial.begin(115200);
  delay(5000);
  
  if( networkManager.loadWifiConfig() &&
      networkManager.initWifiSTA())
  {
    
    networkManager.startWebServerSTA();

  } else {

    networkManager.startWebServerAP();
    
  }
  
  //networkManager.initETH();
  //if(networkManager.isMaster) networkManager.startBroker();
  //networkManager.initMdns();
  //networkManager.initClients();
  
}

void loop()
{
  //networkManager.loop();
}

#include <Arduino.h>
#include <Network/NetworkManager.h>

NetworkManager networkManager;

void setup()
{
  Serial.begin(115200);
  delay(5000);
  
  if( networkManager.loadWifiConfig() && // try to load config
      networkManager.initWifiSTA()) // try to init wifi in station mode
  { 
    networkManager.startWebServer(false);
  } else if ( networkManager.initWifiAP()) // try to init wifi in access point mode
  { 
    networkManager.startWebServer(true); 
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

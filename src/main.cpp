#include <Arduino.h>

#include <Network/NetworkManager.h>
#include <Network/NetworkData.h>
#include <Persistence/PersistenceManager.h>
#include <Hardware/HardwareManager.h>

PersistenceManager persistenceManager;
HardwareManager hardwareManager;

NetworkData networkData(persistenceManager);
NetworkManager networkManager(networkData, hardwareManager);

void setup()
{
  Serial.begin(115200);
  delay(5000);

  hardwareManager.begin();
  networkManager.begin();
  
}

void loop()
{
  hardwareManager.loop();
  networkManager.loop();
}

#include <Arduino.h>

#include <Network/NetManager.h>
#include <Hardware/HardwareManager.h>

HardwareManager hardwareManager;
NetManager netManager(hardwareManager);

void setup()
{
  Serial.begin(115200);
  delay(5000);

  hardwareManager.begin();
  netManager.begin();
  
}

void loop()
{
  hardwareManager.loop();
  netManager.loop();
}

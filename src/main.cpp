#include <Arduino.h>

#include <Network/NetManager.h>
#include <Hardware/HardwareManager.h>

#include <network/NetConfig.h>
#include <TinyMqtt.h> // https://github.com/hsaturn/TinyMqtt

HardwareManager hardwareManager;
NetManager netManager(hardwareManager);

NetConfig netConfig;
MqttBroker broker(1883);
MqttClient mqtt_local(&broker);
MqttClient mqtt_remote;

void onPublish(const MqttClient* , const Topic& topic, const char* payload, size_t )
{ Serial << "--> local client received " << topic.c_str() << ", " << payload << endl; }

const char *ssid     = "CVCATSPOT";
const char *password = "cvc4tsp0t";

void setup()
{
  Serial.begin(115200);
  delay(5000);

  hardwareManager.begin();
  netManager.begin();
  
  /*WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial << '.'; }
  Serial << "Connected to " << ssid << " IP address: " << WiFi.localIP() << endl;*/

  netConfig.init();

  broker.begin();
  
  Serial << "Connecting to mqtt remote ";
  if( netConfig.uniqueHostname == "CV-CATC883EF23A08"){ // green
    while(not mqtt_remote.connected()) {mqtt_remote.connect("192.168.137.12", 1883); Serial << "-"; delay(500);  }
  }
  else { // red
    while(not mqtt_remote.connected()) {mqtt_remote.connect("192.168.137.40", 1883); Serial << "-"; delay(500);  }
  }

  mqtt_local.subscribe("status/update");
  mqtt_remote.subscribe("status/update");

  mqtt_local.setCallback(onPublish);

}

void loop()
{
  //hardwareManager.loop();
  
  broker.loop();
  mqtt_remote.loop();
  mqtt_local.loop();

  static const int intervalA = 5000;  // publishes every 5s (please avoid usage of delay())
  static uint32_t timerA = millis() + intervalA;

  if (millis() > timerA)
  {
    
    timerA += intervalA;
    
    if (not mqtt_remote.connected())
		{
			Serial << millis() << "not connected to remote" << endl;
		}

    if( netConfig.uniqueHostname == "CV-CATC883EF23A08"){ // green
      mqtt_remote.publish("status/update", "sent by green to remote" );
      mqtt_local.publish("status/update", "sent by green to local" );
    }
    else { // red
      mqtt_remote.publish("status/update", "sent by red to remote" );
      mqtt_local.publish("status/update", "sent by red to local" );
    }
    
  }

  

  /*{
    netManager.loop();
  }*/
  
}

#include <Arduino.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <ESPmDNS.h>

#include <Credentials/Credentials.h>
#include <Globals.h>

#include <Network/NetworkManager.h>

NetworkManager networkManager;

/*std::string topic="sensor/temperature";
MqttBroker broker(1883);
MqttClient mqtt_a(&broker);
MqttClient mqtt_b(&broker);

void onPublishA(const MqttClient*, const Topic& topic, const char* payload, size_t )
{ Serial << "--> client A received " << topic.c_str() << ", " << payload << endl; }

void onPublishB(const MqttClient* , const Topic& topic, const char* payload, size_t )
{ Serial << "--> client B Received " << topic.c_str() << ", " << payload << endl; }

bool startBroker = false;*/

void setup()
{
  Serial.begin(115200);
  delay(500);
  
  networkManager.initWifi();
  networkManager.initBroker();
  networkManager.initMdns();
  networkManager.initClients();
  
	/*if (strlen(ssid)==0)
		Serial << "****** RENAME Credentials-Default.h to Credentials.h and add SSID/PASSWORD *************" << endl;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {   Serial << '.'; delay(500); }

  Serial << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;
  Serial << "MAC Address is " << WiFi.macAddress() << endl;

  if (String(BROKER_MAC) == WiFi.macAddress()) startBroker = true;

  if(startBroker){
    Serial << "Starting MDNS";
    if (!MDNS.begin(BROKER_HOSTNAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    Serial << "STARTING BROKER" << endl;
    broker.begin();

    MDNS.addService("mqtt", "tcp", 1883);

    mqtt_a.setCallback(onPublishA);
    mqtt_a.subscribe(topic);

    mqtt_b.setCallback(onPublishB);
    mqtt_b.subscribe(topic);
  }
  else {
    Serial << "Starting MDNS";
    while(mdns_init()!= ESP_OK){ Serial << '.'; delay(1000); }

    IPAddress serverIp;
    Serial << "Resolving Broker IP";
    while (serverIp.toString() == "0.0.0.0") {
      Serial << '.';
      serverIp = MDNS.queryHost(BROKER_HOSTNAME);
      delay(500);
    }
    Serial << "Found broker with IP " << serverIp << endl;

    int nrOfServices = MDNS.queryService("mqtt", "tcp");
    if (nrOfServices == 0) {
      Serial.println("No services were found.");
    } 
    else {
     Serial << "Number of services found: " << nrOfServices << endl;
      for (int i = 0; i < nrOfServices; i=i+1) {
        Serial.println("---------------");
        Serial << "Hostname: " << MDNS.hostname(i) << endl;
        Serial << "IP: " << MDNS.IP(i) << endl;
        Serial << "Port: " << MDNS.port(i) << endl;
      }
    }
  }*/
  
}

void loop()
{
  networkManager.loop();
  
  /*if(startBroker){

    broker.loop();  // Don't forget to add loop for every broker and clients

    mqtt_a.loop();
    mqtt_b.loop();

    // ============= client A publish ================
    static const int intervalA = 5000;  // publishes every 5s (please avoid usage of delay())
    static uint32_t timerA = millis() + intervalA;

    if (millis() > timerA)
    {
      Serial << "A is publishing " << topic.c_str() << endl;
      timerA += intervalA;
      mqtt_a.publish(topic, " sent by A");
    }

    // ============= client B publish ================
    static const int intervalB = 7000;	// will send topic each 7s
    static uint32_t timerB = millis() + intervalB;

    if (millis() > timerB)
    {
      static int temperature;
      Serial << "B is publishing " << topic.c_str() << endl;
      timerB += intervalB;
      mqtt_b.publish(topic, " sent by B: " + std::string(String(16+temperature++%6).c_str()) );
    }
  }*/
}

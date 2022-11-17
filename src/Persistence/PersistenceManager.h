#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

// How to upload data to SPIFFS: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/

#include <Arduino.h>
#include "SPIFFS.h"

class PersistenceManager {
	
	private:
		
		void initSPIFFS();
		
	public:

		// be sure to use exactly the same parameternames in your html files
		String PARAM_WIFI_SSID 		= "wifiSSID";
		String PARAM_WIFI_PWD 		= "wifiPwd";
		String PARAM_WIFI_IP 		= "wifiIP";
		String PARAM_WIFI_GATEWAY 	= "wifiGateway";
		String PARAM_ETH_IP 		= "ethIP";

		PersistenceManager();

		String readFileFromSPIFFS(fs::FS &fs, String path);
        void writeFileToSPIFFS(fs::FS &fs, String path, const char * message);
};


#endif
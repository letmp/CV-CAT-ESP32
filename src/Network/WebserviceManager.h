#ifndef WEBSERVICE_MANAGER_H
#define WEBSERVICE_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Streaming.h>

#include <Hardware/HardwareManager.h>

#include <Network/ConstantProvider.h>
#include <Network/NetworkData.h>

class WebserviceManager
{

private:

	HardwareManager &mHardwareManager;

    NetworkData &mNetworkData;
	AsyncWebServer mAsyncWebServer;
	
	void handleGetNetconfig(AsyncWebServerRequest *request);
	String processTemplateNetconfig(const String &var);
	
	void handlePostNetconfig(AsyncWebServerRequest *request);
	void writeParameterToSPIFFS(AsyncWebParameter *p, String parameter);
	
	void handleGetIO(AsyncWebServerRequest *request);
	void handleGetIOSwitch(AsyncWebServerRequest *request);

public:
	WebserviceManager(NetworkData& networkData, HardwareManager& hardwareManager);
	void begin();
};

#endif
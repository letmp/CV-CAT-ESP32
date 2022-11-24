#ifndef NET_SERVICE_HTTP_H
#define NET_SERVICE_HTTP_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Streaming.h>

#include <Hardware/HardwareManager.h>

#include <Network/NetConstants.h>
#include <Network/NetConfig.h>

class NetServiceHttp
{

private:

	HardwareManager &rHardwareManager;
    NetConfig &rNetConfig;

	AsyncWebServer mAsyncWebServer;
	
	void handleGetNetconfig(AsyncWebServerRequest *request);
	String processTemplateNetconfig(const String &var);
	
	void handlePostNetconfig(AsyncWebServerRequest *request);
	void writeParameterToSPIFFS(AsyncWebParameter *p, String parameter);
	
	void handleGetIO(AsyncWebServerRequest *request);
	void handleGetIOSwitch(AsyncWebServerRequest *request);

public:
	NetServiceHttp(NetConfig& networkData, HardwareManager& hardwareManager);
	void begin();
};

#endif
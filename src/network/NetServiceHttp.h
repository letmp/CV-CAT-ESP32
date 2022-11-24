#ifndef NET_SERVICE_HTTP_H
#define NET_SERVICE_HTTP_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Streaming.h>

#include <hardware/HardwareManager.h>
#include <network/NetConstants.h>
#include <network/NetConfig.h>

class NetServiceHttp
{

private:
	HardwareManager &rHardwareManager;
	NetConfig &rNetConfig;

	AsyncWebServer mAsyncWebServer;

	void handleGetNetconfig(AsyncWebServerRequest *request);
	String processTemplateNetconfig(const String &var);

	void handlePostNetconfig(AsyncWebServerRequest *request);
	
	void handleGetHardware(AsyncWebServerRequest *request);
	void handleGetHardwareSwitch(AsyncWebServerRequest *request);
	void handleGetHardwareData(AsyncWebServerRequest *request);

public:
	NetServiceHttp(NetConfig &networkData, HardwareManager &hardwareManager);
	void begin();
};

#endif
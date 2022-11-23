#include "WebserviceManager.h"
#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

WebserviceManager::WebserviceManager(NetworkData &nd, HardwareManager &hm) : mHardwareManager{hm},
                                                                             mNetworkData{nd},
                                                                             mAsyncWebServer(ConstantProvider::PORT_HTTP)
{
}

void WebserviceManager::begin()
{
    Serial << endl << "--- Starting WebServer ---" << endl;

    mAsyncWebServer.on("/", HTTP_GET, std::bind(&WebserviceManager::handleGetNetconfig, this, std::placeholders::_1));
    mAsyncWebServer.on("/", HTTP_POST, std::bind(&WebserviceManager::handlePostNetconfig, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/", SPIFFS, "/");

    mAsyncWebServer.on("/io/", HTTP_GET, std::bind(&WebserviceManager::handleGetIO, this, std::placeholders::_1));
    mAsyncWebServer.on("/io/switch", HTTP_GET, std::bind(&WebserviceManager::handleGetIOSwitch, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/io/", SPIFFS, "/");

    if (mNetworkData.hasWifiConfig)
        AsyncElegantOTA.begin(&mAsyncWebServer);

    mAsyncWebServer.begin();
    Serial << "http:\\\\" << mNetworkData.uniqueHostname << ".local" << endl;
}

void WebserviceManager::handleGetIO(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/io.html", "text/html");
}
void WebserviceManager::handleGetIOSwitch(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/io.html", "text/html");
}

void WebserviceManager::handleGetNetconfig(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/netconfig.html", String(), false, [this](const String &var) -> String
                  { return this->processTemplateNetconfig(var); });
}

String WebserviceManager::processTemplateNetconfig(const String &var)
{
    if (var == ConstantProvider::NETCONFIG)
    {
        String config = "<b>Your current network settings</b> <br/>";
        config += "WIFI IP: " + mNetworkData.wifiIp.toString() + " / WIFI GATEWAY: " + mNetworkData.wifiIpGateway.toString() + "<br/>";
        config += "ETH IP: " + mNetworkData.ethIp.toString() + " / ETH GATEWAY: " + mNetworkData.ethIpGateway.toString();
        return F(config.c_str());
    }

    if (var == ConstantProvider::WIFI_SSID)
        return F(mNetworkData.wifiSSID.c_str());
    if (var == ConstantProvider::WIFI_PWD)
        return F(mNetworkData.wifiPWD.c_str());

    String ipAddress;
    if (var == ConstantProvider::WIFI_IP_STATIC && mNetworkData.hasStaticWifiAddress)
    {
        ipAddress = mNetworkData.wifiIp.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == ConstantProvider::WIFI_GATEWAY_STATIC && mNetworkData.hasStaticWifiAddress)
    {
        ipAddress = mNetworkData.wifiIpGateway.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == ConstantProvider::ETH_IP_STATIC && mNetworkData.hasStaticEthAddress)
    {
        ipAddress = mNetworkData.ethIp.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == ConstantProvider::ETH_GATEWAY_STATIC && mNetworkData.hasStaticEthAddress)
    {
        ipAddress = mNetworkData.ethIpGateway.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }

    return String();
}

void WebserviceManager::handlePostNetconfig(AsyncWebServerRequest *request)
{
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost())
        {
            if (p->name() == ConstantProvider::WIFI_SSID)
                mNetworkData.wifiSSID = p->value();
            if (p->name() == ConstantProvider::WIFI_PWD)
                mNetworkData.wifiPWD = p->value();
            if (p->name() == ConstantProvider::WIFI_IP_STATIC)
                mNetworkData.wifiIp = IPAddress().fromString(p->value());
            if (p->name() == ConstantProvider::WIFI_GATEWAY_STATIC)
                mNetworkData.wifiIpGateway = IPAddress().fromString(p->value());
            if (p->name() == ConstantProvider::ETH_IP_STATIC)
                mNetworkData.ethIp = IPAddress().fromString(p->value());
            if (p->name() == ConstantProvider::ETH_GATEWAY_STATIC)
                mNetworkData.ethIpGateway = IPAddress().fromString(p->value());
        }
    }
    mNetworkData.writeWifiConfig();
    mNetworkData.writeEthConfig();
    request->send(200, "text/html", "Done. ESP will restart, connect to your router and go to <a href=\"http://" + mNetworkData.uniqueHostname + ".local\">" + mNetworkData.uniqueHostname + ".local</a>");
    delay(3000);
    ESP.restart();
}
#include "NetServiceHttp.h"
#include <AsyncElegantOTA.h> // has to be included here instead of header file -> https://community.platformio.org/t/main-cpp-o-bss-asyncelegantota-0x0-multiple-definition-of-asyncelegantota/26733/3

NetServiceHttp::NetServiceHttp(NetConfig &nd, HardwareManager &hm) : rHardwareManager{hm},
                                                                             rNetConfig{nd},
                                                                             mAsyncWebServer(NetConstants::PORT_HTTP)
{
}

void NetServiceHttp::begin()
{
    Serial << endl << "--- Starting WebServer ---" << endl;

    mAsyncWebServer.on("/", HTTP_GET, std::bind(&NetServiceHttp::handleGetNetconfig, this, std::placeholders::_1));
    mAsyncWebServer.on("/", HTTP_POST, std::bind(&NetServiceHttp::handlePostNetconfig, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/", SPIFFS, "/");

    mAsyncWebServer.on("/io/", HTTP_GET, std::bind(&NetServiceHttp::handleGetIO, this, std::placeholders::_1));
    mAsyncWebServer.on("/io/switch", HTTP_GET, std::bind(&NetServiceHttp::handleGetIOSwitch, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/io/", SPIFFS, "/");

    if (rNetConfig.hasWifiConfig)
        AsyncElegantOTA.begin(&mAsyncWebServer);

    mAsyncWebServer.begin();
    Serial << "http:\\\\" << rNetConfig.uniqueHostname << ".local" << endl;
}

void NetServiceHttp::handleGetIO(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/io.html", "text/html");
}
void NetServiceHttp::handleGetIOSwitch(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/io.html", "text/html");
}

void NetServiceHttp::handleGetNetconfig(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/netconfig.html", String(), false, [this](const String &var) -> String
                  { return this->processTemplateNetconfig(var); });
}

String NetServiceHttp::processTemplateNetconfig(const String &var)
{
    if (var == NetConstants::NETCONFIG)
    {
        String config = "<b>Your current network settings</b> <br/>";
        config += "WIFI IP: " + rNetConfig.wifiIp.toString() + " / WIFI GATEWAY: " + rNetConfig.wifiIpGateway.toString() + "<br/>";
        config += "ETH IP: " + rNetConfig.ethIp.toString() + " / ETH GATEWAY: " + rNetConfig.ethIpGateway.toString();
        return F(config.c_str());
    }

    if (var == NetConstants::WIFI_SSID)
        return F(rNetConfig.wifiSSID.c_str());
    if (var == NetConstants::WIFI_PWD)
        return F(rNetConfig.wifiPWD.c_str());

    String ipAddress;
    if (var == NetConstants::WIFI_IP_STATIC && rNetConfig.hasStaticWifiAddress)
    {
        ipAddress = rNetConfig.wifiIp.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == NetConstants::WIFI_GATEWAY_STATIC && rNetConfig.hasStaticWifiAddress)
    {
        ipAddress = rNetConfig.wifiIpGateway.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == NetConstants::ETH_IP_STATIC && rNetConfig.hasStaticEthAddress)
    {
        ipAddress = rNetConfig.ethIp.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }
    if (var == NetConstants::ETH_GATEWAY_STATIC && rNetConfig.hasStaticEthAddress)
    {
        ipAddress = rNetConfig.ethIpGateway.toString();
        if (ipAddress != "0.0.0.0")
            return F(ipAddress.c_str());
    }

    return String();
}

void NetServiceHttp::handlePostNetconfig(AsyncWebServerRequest *request)
{
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost())
        {
            if (p->name() == NetConstants::WIFI_SSID)
                rNetConfig.wifiSSID = p->value();
            if (p->name() == NetConstants::WIFI_PWD)
                rNetConfig.wifiPWD = p->value();
            if (p->name() == NetConstants::WIFI_IP_STATIC)
                rNetConfig.wifiIp = IPAddress().fromString(p->value());
            if (p->name() == NetConstants::WIFI_GATEWAY_STATIC)
                rNetConfig.wifiIpGateway = IPAddress().fromString(p->value());
            if (p->name() == NetConstants::ETH_IP_STATIC)
                rNetConfig.ethIp = IPAddress().fromString(p->value());
            if (p->name() == NetConstants::ETH_GATEWAY_STATIC)
                rNetConfig.ethIpGateway = IPAddress().fromString(p->value());
        }
    }
    rNetConfig.writeWifiConfig();
    rNetConfig.writeEthConfig();
    request->send(200, "text/html", "Done. ESP will restart, connect to your router and go to <a href=\"http://" + rNetConfig.uniqueHostname + ".local\">" + rNetConfig.uniqueHostname + ".local</a>");
    delay(3000);
    ESP.restart();
}
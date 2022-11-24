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

    mAsyncWebServer.on("/hardware/", HTTP_GET, std::bind(&NetServiceHttp::handleGetHardware, this, std::placeholders::_1));
    mAsyncWebServer.on("/hardware/switch", HTTP_GET, std::bind(&NetServiceHttp::handleGetHardwareSwitch, this, std::placeholders::_1));
    mAsyncWebServer.on("/hardware/data", HTTP_GET, std::bind(&NetServiceHttp::handleGetHardwareData, this, std::placeholders::_1));
    mAsyncWebServer.serveStatic("/hardware/", SPIFFS, "/");

    if (rNetConfig.hasWifiConfig)
        AsyncElegantOTA.begin(&mAsyncWebServer);

    mAsyncWebServer.begin();
    Serial << "http:\\\\" << rNetConfig.uniqueHostname << ".local" << endl;
}

void NetServiceHttp::handleGetNetconfig(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/netconfig.html", String(), false, [this](const String &var) -> String { return this->processTemplateNetconfig(var); });
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
            if (p->name() == NetConstants::WIFI_IP_STATIC){
                if(p->value() != "") rNetConfig.wifiIp.fromString(p->value()); //try to set new ip if given
                else rNetConfig.wifiIp.fromString("0.0.0.0"); // else reset ip so that DHCP is used after reboot
            }
            if (p->name() == NetConstants::WIFI_GATEWAY_STATIC){
                if(p->value() != "") rNetConfig.wifiIpGateway.fromString(p->value());
                else rNetConfig.wifiIpGateway.fromString("0.0.0.0");
            }
            if (p->name() == NetConstants::ETH_IP_STATIC){
                if(p->value() != "") rNetConfig.ethIp.fromString(p->value());
                else rNetConfig.ethIp.fromString("0.0.0.0");
            }
            if (p->name() == NetConstants::ETH_GATEWAY_STATIC){
                if(p->value() != "") rNetConfig.ethIpGateway.fromString(p->value());
                else rNetConfig.ethIpGateway.fromString("0.0.0.0");
            }
        }
    }
    rNetConfig.writeWifiConfig();
    rNetConfig.writeEthConfig();

    request->send(200, "text/html", "Done. ESP will restart, connect to your router and go to <a href=\"http://" + rNetConfig.uniqueHostname + ".local\">" + rNetConfig.uniqueHostname + ".local</a>");
    delay(3000);
    ESP.restart();
}

void NetServiceHttp::handleGetHardware(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/hardware.html", "text/html");
}
void NetServiceHttp::handleGetHardwareSwitch(AsyncWebServerRequest *request)
{
    int newVal = 0;
    if(rHardwareManager.jacksOut.jack1 == 0) newVal = 4095;
    rHardwareManager.jacksOut = (JacksOut){.jack1 = newVal, .jack2 = newVal};
    request->send(200, "text/plain", "switched");
}
void NetServiceHttp::handleGetHardwareData(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    JsonArray buttons = json.createNestedArray("buttons");
    JsonArray jacks = json.createNestedArray("jacks");
    JsonArray pots = json.createNestedArray("pots");
    buttons.add(rHardwareManager.buttons.button1);
    buttons.add(rHardwareManager.buttons.button2);
    jacks.add(rHardwareManager.jacksIn.jack1);
    pots.add(rHardwareManager.pots.pot1);
    pots.add(rHardwareManager.pots.pot2);
    serializeJson(json, *response);
    request->send(response);
}

#include "NetConfig.h"

NetConfig::NetConfig()
{
    char buffer[23];
    snprintf(buffer, 23, "%s%llX", NetConstants::DEVICENAME, ESP.getEfuseMac());
    Serial << endl << "--- Setting unique hostname [" << buffer << "] ---" << endl;
    uniqueHostname = buffer;

    hasWifiConfig = loadWifiConfig();
    hasEthConfig = loadEthConfig();
}

bool NetConfig::loadWifiConfig()
{
    Serial << endl << "--- Loading WIFI Config---" << endl;

    wifiSSID = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::WIFI_SSID);
    wifiPWD = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::WIFI_PWD);
    String wifiIpStr = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::WIFI_IP_STATIC);
    String wifiIpGatewayStr = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::WIFI_GATEWAY_STATIC);

    if (wifiSSID == "")
    {
        Serial << "Undefined SSID. Starting Access Point." << endl;
        return false;
    }

    if (wifiIpStr != "" || wifiIpGatewayStr != "")
    {
        hasStaticWifiAddress =
            wifiIp.fromString(wifiIpStr.c_str()) &&
            wifiIpGateway.fromString(wifiIpGatewayStr.c_str());
    }
    return true;
}

bool NetConfig::loadEthConfig()
{
    Serial << endl << "--- Loading ETH Config---" << endl;

    String ethIpStr = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::ETH_IP_STATIC);
    String ethIpGatewayStr = mPersistenceUtils.readFileFromSPIFFS(SPIFFS, "/" + NetConstants::ETH_GATEWAY_STATIC);

    if (ethIpStr != "" || ethIpGatewayStr != "")
    {
        hasStaticEthAddress =
            ethIp.fromString(ethIpStr.c_str()) &&
            ethIpGateway.fromString(ethIpGatewayStr.c_str());
    }

    return true;
}

void NetConfig::writeWifiConfig(){
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::WIFI_SSID, wifiSSID.c_str());
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::WIFI_PWD, wifiPWD.c_str());
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::WIFI_IP_STATIC, wifiIp.toString().c_str());
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::WIFI_GATEWAY_STATIC, wifiIpGateway.toString().c_str());
}

void NetConfig::writeEthConfig(){
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::ETH_IP_STATIC, ethIp.toString().c_str());
    mPersistenceUtils.writeFileToSPIFFS(SPIFFS, "/" + NetConstants::ETH_GATEWAY_STATIC, ethIpGateway.toString().c_str());
}
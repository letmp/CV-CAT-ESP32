#include "NetworkData.h"

NetworkData::NetworkData(PersistenceManager &pm) : mPersistenceManager{pm}
{
    char buffer[23];
    snprintf(buffer, 23, "%s%llX", ConstantProvider::DEVICENAME, ESP.getEfuseMac());
    Serial << endl << "--- Setting unique hostname [" << buffer << "] ---" << endl;
    uniqueHostname = buffer;

    hasWifiConfig = loadWifiConfig();
    hasEthConfig = loadEthConfig();
}

bool NetworkData::loadWifiConfig()
{
    Serial << endl << "--- Loading WIFI Config---" << endl;

    wifiSSID = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_SSID);
    wifiPWD = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_PWD);
    String wifiIpStr = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_IP_STATIC);
    String wifiIpGatewayStr = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_GATEWAY_STATIC);

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

bool NetworkData::loadEthConfig()
{
    Serial << endl << "--- Loading ETH Config---" << endl;

    String ethIpStr = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::ETH_IP_STATIC);
    String ethIpGatewayStr = mPersistenceManager.readFileFromSPIFFS(SPIFFS, "/" + ConstantProvider::ETH_GATEWAY_STATIC);

    if (ethIpStr != "" || ethIpGatewayStr != "")
    {
        hasStaticEthAddress =
            ethIp.fromString(ethIpStr.c_str()) &&
            ethIpGateway.fromString(ethIpGatewayStr.c_str());
    }

    return true;
}

void NetworkData::writeWifiConfig(){
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_SSID, wifiSSID.c_str());
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_PWD, wifiPWD.c_str());
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_IP_STATIC, wifiIp.toString().c_str());
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::WIFI_GATEWAY_STATIC, wifiIpGateway.toString().c_str());
}

void NetworkData::writeEthConfig(){
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::ETH_IP_STATIC, ethIp.toString().c_str());
    mPersistenceManager.writeFileToSPIFFS(SPIFFS, "/" + ConstantProvider::ETH_GATEWAY_STATIC, ethIpGateway.toString().c_str());
}
#include "PersistenceManager.h"

PersistenceManager::PersistenceManager() {	
    initSPIFFS();
}

void PersistenceManager::initSPIFFS() {
  Serial.println("--- Mounting SPIFFS---");
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
}

String PersistenceManager::readFileFromSPIFFS(fs::FS &fs, String path){
  Serial.printf("Reading file: %s\r\n", path.c_str());

  File file = fs.open(path.c_str());
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

void PersistenceManager::writeFileToSPIFFS(fs::FS &fs, String path, const char * message){
  Serial.printf("Writing file: %s\r\n", path.c_str());
  Serial.printf("Content: %s\r\n", message);

  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}
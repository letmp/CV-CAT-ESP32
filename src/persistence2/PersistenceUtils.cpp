#include "PersistenceUtils.h"

PersistenceUtils::PersistenceUtils()
{
  initSPIFFS();
}

void PersistenceUtils::initSPIFFS()
{
  Serial << "--- Mounting SPIFFS---" << endl;
  if (!SPIFFS.begin(true))
  {
    Serial << "An error has occurred while mounting SPIFFS" << endl;
  }
}

String PersistenceUtils::readFileFromSPIFFS(fs::FS &fs, String path)
{
  Serial << "Try to read file [" << path.c_str() << "] ";

  File file = fs.open(path.c_str());
  if (!file || file.isDirectory())
  {
    Serial << "- Not found" << endl;
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  Serial << endl;
  return fileContent;
}

void PersistenceUtils::writeFileToSPIFFS(fs::FS &fs, String path, const char *message)
{
  Serial << "Writing file [" << path.c_str() << "] with content [" << message << "]";
  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file)
  {
    Serial << "- ERROR: Failed to open" << endl;
    ;
    return;
  }
  if (!file.print(message))
  {
    Serial << "- ERROR: Failed to write";
  }
  Serial << endl;
}

void PersistenceUtils::removeFiles()
{
  Serial << "Removing all files from SPIFFS" << endl;
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    SPIFFS.remove(file.name());
    file = root.openNextFile();
  }
}
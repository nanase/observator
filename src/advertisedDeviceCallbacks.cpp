#include "advertisedDeviceCallbacks.h"

#include <Arduino.h>

#include "time.h"

void AdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
  if (advertisedDevice.haveServiceUUID() || !advertisedDevice.haveServiceData() ||
      !advertisedDevice.haveManufacturerData())
    return;

  if (!advertisedDevice.getServiceDataUUID().equals(this->serviceDataUUID))
    return;

  std::string serviceDataStr      = advertisedDevice.getServiceData();
  std::string manufacturerDataStr = advertisedDevice.getManufacturerData();
  size_t serviceDataSize          = serviceDataStr.size();
  size_t manufacturerDataSize     = manufacturerDataStr.size();
  const char* serviceData         = serviceDataStr.c_str();
  const char* manufacturerData    = manufacturerDataStr.c_str();

  if (serviceDataSize != 3 || manufacturerDataSize != 14)
    return;

  // 防水温湿度計 Model: W3400010 (0x77)
  if (serviceData[0] != 0x77)
    return;

  // vendor id (0x0969)
  if (manufacturerData[0] != 0x69 || manufacturerData[1] != 0x09)
    return;

  int8_t battery                  = serviceData[2] & 0b01111111;
  bool isTemperatureAboveFreezing = manufacturerData[11] & 0b10000000;
  float temperature               = (manufacturerData[10] & 0b00001111) / 10.0 + (manufacturerData[11] & 0b01111111);
  int8_t humidity                 = manufacturerData[12] & 0b01111111;

  if (!isTemperatureAboveFreezing)
    temperature = -temperature;

  struct tm datetime;
  getLocalTime(&datetime);
  ObservationResult result = { this->sequenceNumber, (int8_t)advertisedDevice.getRSSI(), battery, temperature, humidity,
                               mktime(&datetime) };
  this->observationResults[advertisedDevice.getAddress()] = result;
  this->sequenceNumber++;
}

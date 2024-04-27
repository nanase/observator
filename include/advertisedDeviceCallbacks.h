#pragma once

#include <BLEUtils.h>

#include "observationResult.h"

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
 public:
  AdvertisedDeviceCallbacks(std::map<BLEAddress, ObservationResult>& observationResults)
      : observationResults(observationResults) {}

  void onResult(BLEAdvertisedDevice advertisedDevice);

 private:
  uint32_t sequenceNumber = 0;
  std::map<BLEAddress, ObservationResult>& observationResults;
  BLEUUID serviceDataUUID = BLEUUID("0000fd3d-0000-1000-8000-00805f9b34fb");
};

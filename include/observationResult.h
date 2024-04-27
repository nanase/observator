#pragma once

struct ObservationResult {
  uint32_t sequenceNumber;
  int8_t rssi;
  int8_t battery;
  float temperature;
  int8_t humidity;
  time_t fetched_at;
};

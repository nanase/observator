#include "ntp.h"

#include <Arduino.h>

#include "config.h"
#include "esp_sntp.h"
#include "time.h"

const long gmtOffset_sec     = 9 * 3600;
const int daylightOffset_sec = 0;

void onSetTimeSync(struct timeval* t) {
  Serial.print("Got time adjustment from NTP: ");
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "%F %T (%z, %Z)");
}

void setupNTPSync() {
  sntp_set_time_sync_notification_cb(onSetTimeSync);
  configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  struct tm timeinfo;
  Serial.println("Time adjust from NTP ");

  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
}

#include "httpServer.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <WebServer.h>
#include <esp_mac.h>

#include "Adafruit_BME280.h"
#include "mh-z19c-pwm.h"
#include "observationResult.h"

extern uint32_t observationSequenceNumber;
extern std::map<BLEAddress, ObservationResult> observationResults;
extern Adafruit_BME280 bme;
extern MH_Z19C_PWM<12> co2sensor;

WebServer server(80);

void handleObservationResult() {
  String json = "{";
  json += "\"sequence\":" + String(observationSequenceNumber) + ",";
  json += "\"result\":[";

  bool continued = false;
  for (auto x = observationResults.begin(); x != observationResults.end(); x++) {
    BLEAddress address       = x->first;
    ObservationResult result = x->second;

    if (continued)
      json += ",";

    json += "{";
    json += "\"address\":\"" + String(address.toString().c_str()) + "\",";
    json += "\"type\":\"W3200010\",";
    json += "\"sequence\":" + String(result.sequenceNumber) + ",";
    json += "\"fetchedAt\":" + String(result.fetched_at) + ",";
    json += "\"sensor\":[";
    {
      json += "{\"type\":\"temperature\",";
      json += "\"value\":" + String(result.temperature, 1) + ",";
      json += "\"unit\":\"℃\",";
      json += "\"precision\":1},";

      json += "{\"type\":\"humidity\",";
      json += "\"value\":" + String(result.humidity) + ",";
      json += "\"unit\":\"%\",";
      json += "\"precision\":0},";

      json += "{\"type\":\"battery\",";
      json += "\"value\":" + String(result.battery) + ",";
      json += "\"unit\":\"%\",";
      json += "\"precision\":0},";

      json += "{\"type\":\"rssi\",";
      json += "\"value\":" + String(result.rssi) + ",";
      json += "\"unit\":\"dBm\",";
      json += "\"precision\":0}";
    }
    json += "]";
    json += "}";

    continued = true;
  }

  if (continued)
    json += ",";

  {
    BLEAddress macAddress("00:00:00:00:00:00");
    esp_read_mac((uint8_t *)macAddress.getNative(), ESP_MAC_BT);
    struct tm datetime;
    getLocalTime(&datetime);

    json += "{";
    json += "\"address\":\"" + String(macAddress.toString().c_str()) + "\",";
    json += "\"type\":\"ESP32-Central\",";
    json += "\"sequence\":" + String(observationSequenceNumber) + ",";
    json += "\"fetchedAt\":" + String(mktime(&datetime)) + ",";
    json += "\"sensor\":[";
    {
      json += "{\"type\":\"temperature\",";
      json += "\"value\":" + String(bme.readTemperature(), 2) + ",";
      json += "\"unit\":\"℃\",";
      json += "\"precision\":2},";

      json += "{\"type\":\"humidity\",";
      json += "\"value\":" + String(bme.readHumidity(), 3) + ",";
      json += "\"unit\":\"%\",";
      json += "\"precision\":3},";

      json += "{\"type\":\"pressure\",";
      json += "\"value\":" + String(bme.readPressure() / 100.0, 4) + ",";
      json += "\"unit\":\"hPa\",";
      json += "\"precision\":4},";

      json += "{\"type\":\"co2\",";
      json += "\"value\":" + String(co2sensor.getPPM()) + ",";
      json += "\"unit\":\"ppm\",";
      json += "\"precision\":0}";
    }
    json += "]";
    json += "}";
  }

  json += "]";
  json += "}";

  server.send(200, "application/json", json);
}

void setupHTTPServer() {
  server.on("/observation/result", handleObservationResult);
  server.enableCORS(true);
  server.begin();
  Serial.println("HTTP server started");
}

#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_task_wdt.h>

#include "Adafruit_BME280.h"
#include "advertisedDeviceCallbacks.h"
#include "config.h"
#include "httpServer.h"
#include "mh-z19c-pwm.h"
#include "ntp.h"
#include "observationResult.h"
#include "time.h"

const IPAddress ip(WIFI_IPADDR);
const IPAddress gateway(WIFI_GATEWAY);
const IPAddress subnet(WIFI_SUBNET);
const IPAddress dns1(WIFI_DNS1);
const IPAddress dns2(WIFI_DNS2);
extern WebServer server;
MH_Z19C_PWM<12> co2sensor;
Adafruit_BME280 bme;
BLEScan *pBLEScan;
std::map<BLEAddress, ObservationResult> observationResults;
uint32_t observationSequenceNumber = 0;

void setup() {
  Serial.begin(115200);

  // connect to WiFi
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.config(ip, gateway, subnet, dns1, dns2);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  setupNTPSync();
  setupHTTPServer();
  co2sensor.begin();
  Wire1.begin(33, 32);

  if (!bme.begin(BME280_ADDRESS, &Wire1)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1)
      delay(1);
  }

  bme.setSampling(Adafruit_BME280::MODE_NORMAL, Adafruit_BME280::SAMPLING_X16, Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X16, Adafruit_BME280::FILTER_X8, Adafruit_BME280::STANDBY_MS_20);

  // BLE Scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks(observationResults), true);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(BLE_SCAN_INTERVAL);
  pBLEScan->setWindow(BLE_SCAN_WINDOW);

  esp_task_wdt_init(10, true);  // enable panic so ESP32 restarts, interrupt when task executed for more than 3 secons
  esp_task_wdt_add(NULL);       // add current thread to WDT watch

  Serial.println("Ready.");
}

void loop() {
  observationResults.clear();
  pBLEScan->start(BLE_SCAN_DURATION, false);
  pBLEScan->clearResults();

  for (auto x = observationResults.begin(); x != observationResults.end(); x++) {
    BLEAddress address       = x->first;
    ObservationResult result = x->second;
    printf("#%ld %ld [%s] %d dBm, Bat: %d %%, Tmp: %.1f C, Hum: %d %%\n", result.sequenceNumber, result.fetched_at,
           address.toString().c_str(), result.rssi, result.battery, result.temperature, result.humidity);
  }

  observationSequenceNumber++;
  printf("Tmp: %.1f C, Hum: %.1f %%, Prs: %.1f hPa, CO2: %d ppm\n", bme.readTemperature(), bme.readHumidity(),
         bme.readPressure() / 100.0, co2sensor.getPPM());

  for (size_t t = 0; t < 300; t++) {
    server.handleClient();
    esp_task_wdt_reset();
    delay(100);
  }
}

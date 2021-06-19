#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "arduino_secrets.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

/*  WIFI CREDENTIALS: these must be set in arduino_secrets.h, which must
 *  be located in the same directory as this sketch. It we ignore this file
 *  in git because we don't want our wifi password saved in version control.
 */
const char* ssid     = WIFI_NETWORK_NAME;
const char* password = WIFI_PASSWORD;

/*
 * Edit SLEEP_SECONDS to choose how long you want the ESP32 to sleep before
 * posting another reading.
 *
 */
#define SLEEP_SECONDS 3

#define LONG_SLEEP_MS SLEEP_SECONDS * 1000

String payload;

void connectWifi()
{
    // in case we are reconnecting, we need to disconnect first.
    WiFi.disconnect();

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

void initializeSensor() {
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

}

void setup()
{
    Serial.begin(115200);
    delay(10);

    initializeSensor();
    connectWifi();
    
}

const String HTTP_POST_ENDPOINT = String("http://") + String(SERVER_IP_ADDR) + String("/sensor-data/");

void postData(String name, float measured) {
    const int capacity = JSON_OBJECT_SIZE(6);
    StaticJsonDocument<capacity> doc;
    doc["sensor_mac_addr"] = WiFi.macAddress();
    doc["numeric_value"] = measured;
    doc["sensor_id"] = WiFi.macAddress() + ":" + name;
    String str;
    serializeJson(doc, str);

    HTTPClient http;

    http.begin(HTTP_POST_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(str);

    Serial.println("code ... " + String(httpCode));

    if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_CREATED) {
            const String& payload = http.getString();
            Serial.println("received payload:\n<<");
            Serial.println(payload);
            Serial.println(">>");
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
            const String& payload = http.getString();
            Serial.println(payload);
        }
    }
    http.end();
}

void loop()
{
    delay(1000);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi was disconnected ... reconnecting");
      connectWifi();
    } else {
      Serial.println("WiFi still connected");
    }

    if (! bme.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
    Serial.print("Temperature = ");
    Serial.print(bme.temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.humidity);
    Serial.println(" %");

    Serial.print("Gas = ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(" KOhms");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.println();

    Serial.println("Sleeping...");
    delay(LONG_SLEEP_MS);
    Serial.println("Waking up ...");
}

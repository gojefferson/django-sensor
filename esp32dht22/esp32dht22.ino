#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <DHT.h>;
#include "arduino_secrets.h"

/*  WIFI CREDENTIALS: these must be set in arduino_secrets.h, which must
 *  be located in the same directory as this sketch. It we ignore this file
 *  in git because we don't want our wifi password saved in version control.
 */
const char* ssid     = WIFI_NETWORK_NAME;
const char* password = WIFI_PASSWORD;

/*
 * Select the pin on the ESP32 that is connected to the data pin
 * on the DHT 22. This is usually a yellow wire, though some tutorials
 * show a different colored wire.
 *
 * Here are some tutorials for connecting the DHT 22 to an ESP32:
 * - https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
 * - https://learn.adafruit.com/dht/connecting-to-a-dhtxx-sensor
 *
 * Note: some DHT 22 sensors do not seem to need a pull up resistor or else have
 * one pre-installed.
 */
#define DHTPIN 5     // what pin we're connected to

/*
 * Edit SLEEP_SECONDS to choose how long you want the ESP32 to sleep before
 * posting another reading.
 *
 */
#define SLEEP_SECONDS 120 // sleep for 2 minutes

#define LONG_SLEEP_MS SLEEP_SECONDS * 1000
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

float hum;  //Stores humidity value
float temp; //Stores temperature value
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

void setup()
{
    Serial.begin(115200);
    delay(10);
    dht.begin();

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

    hum = dht.readHumidity();
    temp= dht.readTemperature(true);

    postData("hum", hum);
    postData("temp", temp);
    Serial.println("Sleeping...");
    delay(LONG_SLEEP_MS);
    Serial.println("Waking up ...");
}

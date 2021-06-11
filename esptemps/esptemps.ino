#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <DHT.h>;

const char* ssid     = "<WIFI_NETWORK_NAME>";
const char* password = "<WIFI_PASSWORD>";

#define DHTPIN 5     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

float hum;  //Stores humidity value
float temp; //Stores temperature value
String payload;

void setup()
{
    Serial.begin(115200);
    delay(10);
    dht.begin();

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


void postData(String name, float measured) {
    const int capacity = JSON_OBJECT_SIZE(6);
    StaticJsonDocument<capacity> doc;
    doc["sensor_mac_addr"] = WiFi.macAddress();
    doc["numeric_value"] = measured;
    doc["sensor_id"] = WiFi.macAddress() + ":" + name;
    String str;
    serializeJson(doc, str);

    HTTPClient http;

    http.begin("http://<SERVER_IP_ADDRESS/sensor-data/"); //HTTP
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

#define LONG_SLEEP_MS 5 * 60 * 1000  // five minutes

void loop()
{
    delay(1000);

    hum = dht.readHumidity();
    temp= dht.readTemperature(true);

    postData("hum", hum);
    postData("temp", temp);
    Serial.println("Sleeping...");
    delay(LONG_SLEEP_MS);
    Serial.println("Waking up ...");
}

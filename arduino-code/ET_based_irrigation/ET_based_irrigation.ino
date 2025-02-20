#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <ArduinoJson.h>


// WiFi Credentials
const char* ssid = "Ranger";
const char* password = "rangerdale";

// API details
const char server[] = "https://api.weatherlink.com";
const int port = 443;
const char endpoint[] = "/v2/current/fa7242b0-42f0-4479-8776-39bff6f88fa3?api-key=wcdxcltdyrot1pzkrurgefattlsg9sbr";
const char apiSecret[] = "apyephcpeehts3ygbnqmnzteqamn3jua";

// RTC Instance
RTC_DS3231 rtc;

// WiFi and HTTP Clients
WiFiClient wifi;
HttpClient client(wifi, server, port);

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");

    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC. Check wiring!");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
    }
}

void fetchWeatherData() {
    Serial.println("Sending GET request...");
    
    client.beginRequest();
    client.get(endpoint);
    client.sendHeader("X-Api-Secret", apiSecret);
    client.endRequest();

    int statusCode = client.responseStatusCode();
    Serial.print("Status Code: ");
    Serial.println(statusCode);

    if (statusCode == 200) {
        String payload = client.responseBody();
        Serial.println("Response:");
        Serial.println(payload);
        // Declare JSON document correctly
        DynamicJsonDocument doc(4096);
        
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("JSON Parsing Error: ");
            Serial.println(error.c_str());
            return;
        }

        // Parse JSON correctly
        JsonArray sensors = doc["sensors"].as<JsonArray>();
        for (JsonObject sensor : sensors) {
            if (sensor["lsid"] == 811158) {
                float heat_index = sensor["heat_index"] | 0.0;
                float hum_out = sensor["hum_out"] | 0.0;
                float et_day = sensor["et_day"] | 0.0;
                float rain_day_in = sensor["rain_day_in"] | 0.0;
                float temp_out = sensor["temp_out"] | 0.0;

                Serial.print("Heat Index: ");
                Serial.println(heat_index);
                Serial.print("Humidity Outside: ");
                Serial.println(hum_out);
                Serial.print("ET Day: ");
                Serial.println(et_day);
                Serial.print("Rain Day (in): ");
                Serial.println(rain_day_in);
                Serial.print("Temperature Outside: ");
                Serial.println(temp_out);
            }
        }
    } else {
        Serial.print("HTTP Error Code: ");
        Serial.println(statusCode);
    }
    client.stop();
}


void loop() {
    fetchWeatherData();
    delay(120*1000);  // Refresh every 60 seconds
}

#include "Arduino.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

static const char *TAG = "Database Test";
JsonDocument Data;

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true, "/minidb")) {
        ESP_LOGE(TAG, "Spiffs failed to mount.");
    } else
        ESP_LOGI(TAG, "Spiffs mounted successfully.");

    File database = SPIFFS.open("/database.json", FILE_READ);
    if (!database) {
        ESP_LOGE(TAG, "File failed to open.");
    } else
        ESP_LOGI(TAG, "File opened successfully.");

    ESP_LOGI(TAG, "Filesize in bytes: %d", database.size());

    deserializeJson(Data, database);
    database.close();

    ESP_LOGI(TAG, "File content test examples:");

    Serial.println("Example of Data[0][0][\"subject\"]:");
    String Afdeling = Data[0][0]["subject"];
    Serial.println(Afdeling);

    Serial.println("Example of Data[0][1][\"name\"]:");
    String Name = Data[0][1]["name"];
    Serial.println(Name);

    Serial.print("Input new JSON: ");
}

void loop() {
    while (Serial.available()) {
        deserializeJson(Data, Serial);
        if(!Data.isNull()) {
            Serial.println("New Data: ");
            serializeJson(Data, Serial);

            File database = SPIFFS.open("/database.json", FILE_WRITE);
            serializeJson(Data, database);
            database.close();
        }
    }
}
#include <Arduino.h>
#include <ATTinyBME280.h>

ATTinyBME280 sensor(0x76);
bool sensorInitialized = false;

void setup() {
    Serial.begin(9600);
    Serial.println(F("ATTinyBME280 serial example"));

    sensorInitialized = sensor.begin(25);
    if (!sensorInitialized) {
        Serial.println(F("BME280 not found"));
    }
}

void loop() {
    if (!sensorInitialized) {
        sensorInitialized = sensor.begin(25);
        if (sensorInitialized) {
            Serial.println(F("BME280 initialized"));
        }
    } else if (!sensor.isConnected()) {
        sensorInitialized = false;
        Serial.println(F("BME280 disconnected"));
    } else if (sensor.readData()) {
        Serial.print(F("Temperature: "));
        Serial.print(sensor.temperature, 2);
        Serial.println(F(" C"));

        Serial.print(F("Pressure:    "));
        Serial.print(sensor.pressure, 2);
        Serial.println(F(" hPa"));

        Serial.print(F("Humidity:    "));
        Serial.print(sensor.humidity, 2);
        Serial.println(F(" %"));
    } else {
        sensorInitialized = false;
        Serial.println(F("Measurement failed"));
    }

    delay(3000);
}

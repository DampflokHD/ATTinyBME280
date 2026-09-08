#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ATTinyBME280.h>

// RX is unused; TX is PB3 so PB0 remains available for I2C SDA.
SoftwareSerial mySerial(4, 3);
ATTinyBME280 sensor(0x76);
bool sensorInitialized = false;

void setup() {
    mySerial.begin(9600);
    mySerial.println(F("ATTinyBME280 serial example"));

    sensorInitialized = sensor.begin(25);
    if (!sensorInitialized) {
        mySerial.println(F("BME280 not found"));
    }
}

void loop() {
    if (!sensorInitialized) {
        sensorInitialized = sensor.begin(25);
        if (sensorInitialized) {
            mySerial.println(F("BME280 initialized"));
        }
    } else if (!sensor.isConnected()) {
        sensorInitialized = false;
        mySerial.println(F("BME280 disconnected"));
    } else if (sensor.readData()) {
        mySerial.print(F("Temperature: "));
        mySerial.print(sensor.temperature, 2);
        mySerial.println(F(" C"));

        mySerial.print(F("Pressure:    "));
        mySerial.print(sensor.pressure, 2);
        mySerial.println(F(" hPa"));

        mySerial.print(F("Humidity:    "));
        mySerial.print(sensor.humidity, 2);
        mySerial.println(F(" %"));
    } else {
        sensorInitialized = false;
        mySerial.println(F("Measurement failed"));
    }

    delay(3000);
}

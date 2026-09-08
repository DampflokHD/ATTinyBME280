#include <ATTinyBME280.h>

ATTinyBME280 sensor(0x76);
bool sensorInitialized = false;

void setup() {
    sensorInitialized = sensor.begin(25);
}

void loop() {
    if (!sensorInitialized) {
        sensorInitialized = sensor.begin(25);
    } else if (!sensor.isConnected()) {
        sensorInitialized = false;
    } else if (!sensor.readData()) {
        sensorInitialized = false;
    }

    // Read the public values after a successful readData() call:
    // sensor.temperature, sensor.pressure, sensor.humidity
    delay(3000);
}

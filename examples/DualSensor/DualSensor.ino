#include <ATTinyBME280.h>

ATTinyBME280 sensorGnd(0x76); // SDO connected to GND
ATTinyBME280 sensorVcc(0x77); // SDO connected to VCC

bool sensorGndInitialized = false;
bool sensorVccInitialized = false;

void updateSensor(ATTinyBME280& sensor, bool& initialized) {
    if (!initialized) {
        initialized = sensor.begin(25);
        return;
    }

    if (!sensor.isConnected() || !sensor.readData()) {
        initialized = false;
    }
}

void setup() {
    sensorGndInitialized = sensorGnd.begin(25);
    sensorVccInitialized = sensorVcc.begin(25);
}

void loop() {
    updateSensor(sensorGnd, sensorGndInitialized);
    updateSensor(sensorVcc, sensorVccInitialized);

    if (sensorGndInitialized) {
        // Use sensorGnd.temperature, pressure and humidity here.
    }

    if (sensorVccInitialized) {
        // Use sensorVcc.temperature, pressure and humidity here.
    }

    delay(3000);
}

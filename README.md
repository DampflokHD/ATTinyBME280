# ATTinyBME280

A lightweight BME280 library for ATtiny25, ATtiny45 and ATtiny85 microcontrollers.
The library uses the ATtiny USI peripheral as a software-controlled I2C master and
has no dependency on `Arduino.h`.

## Features

- Temperature, pressure and relative humidity readings
- BME280 calibration and Bosch compensation formulas
- Forced-mode measurements with x1 oversampling
- Configurable I2C clock up to 100 kHz
- I2C ACK/NACK and clock-stretching timeouts
- Limited I2C bus recovery with up to nine SCL pulses
- Low RAM usage and 32-bit compensation arithmetic

## Installation

### Arduino IDE
1. Download this repository as a `.zip` file.
2. Open the Arduino IDE.
3. Navigate to **Sketch** > **Include Library** > **Add .ZIP Library...**
4. Select the downloaded `.zip` archive.

### Installation via Arduino Library Manager
You can easily install this library using the built-in Library Manager of the Arduino IDE. 

1. Open the **Arduino IDE**.
2. Navigate to **Tools** → **Manage Libraries...** (or click the Library Manager icon on the left sidebar in IDE 2.x).
3. Search for **`ATTinyBME280`**.
4. Find the library by **`DampflokHD`** and click **Install**.

### PlatformIO

Copy the library into the project's `lib` directory or install it from its Git
repository after publishing. The library itself does not require `Arduino.h`,
but the surrounding application may use an Arduino-compatible framework.

## Supported hardware

The implementation currently targets ATtiny25, ATtiny45 and ATtiny85 devices with
USI hardware.

The I2C pins are fixed by the implementation:

| Signal | ATtiny pin |
|---|---|
| SDA | PB0 |
| SCL | PB2 |

Also connect:

| BME280 signal | Connection |
|---|---|
| VCC | Suitable sensor supply, typically 3.3 V |
| GND | ATtiny GND |
| CSB | VCC for I2C mode |
| SDO | GND for `0x76`, VCC for `0x77` |

Use external pull-up resistors on SDA and SCL. A value around 4.7 kOhm is a common
starting point, depending on bus voltage, wiring and bus capacitance.

The BME280 must be configured for I2C mode. Connect CSB to VCC. The I2C address is
selected with SDO:

- SDO to GND: `0x76`
- SDO to VCC: `0x77`

Do not disconnect the power or ground of a sensor while it remains connected to the
shared SDA/SCL bus. An unpowered sensor can load the bus through its protection
structures. Use a bus switch or proper power and signal isolation when hot-plugging
or independently powering sensors is required.

## How it works

The ATtiny USI peripheral is configured as an I2C master. The library generates the
SCL clock by software strobes and uses the USI shift register for each data byte.
This keeps the implementation independent of `Arduino.h`, but it also means that
the bus pins and the supported MCU family are fixed.

On `begin()`, the library waits for sensor power-up, initializes the USI hardware,
checks the BME280 chip ID, reads the factory calibration registers and configures
forced mode with x1 oversampling. Calibration is required before valid pressure and
humidity compensation can be calculated.

Each `readData()` call starts one forced conversion, waits for the x1 conversion to
finish, reads the pressure, temperature and humidity registers, and applies the
Bosch integer compensation formulas. The public float values are updated only after
the bus read and raw-value validation succeed.

## Sensor lifecycle and recovery

`begin()` is the initialization operation, not just an address scan. If a sensor is
missing at startup, the application can call `begin()` again later. If an initialized
sensor stops responding, call `isConnected()` and mark it uninitialized before
retrying `begin()`; this reloads calibration when the sensor returns.

The driver checks ACK/NACK responses and uses bounded waits for SCL. If a transfer
is interrupted and a device leaves SDA low, the driver temporarily disables USI,
generates up to nine SCL pulses, sends a STOP condition and restores USI mode. This
can recover a slave that is stuck in the middle of a transaction. It cannot repair a
missing power connection, a permanent electrical short, or an unpowered sensor that
loads the shared bus.

## Usage

A complete Arduino IDE example is available in
[`examples/BasicRead/BasicRead.ino`](examples/BasicRead/BasicRead.ino).
For two BME280 devices with addresses `0x76` and `0x77`, see
[`examples/DualSensor/DualSensor.ino`](examples/DualSensor/DualSensor.ino).
For a quick serial monitor test using the Arduino `Serial` API, see
[`examples/SerialMonitor/SerialMonitor.ino`](examples/SerialMonitor/SerialMonitor.ino).
This example depends on the serial implementation provided by the selected
Arduino core; the BME280 library itself still does not require `Arduino.h`.

```cpp
#include <ATTinyBME280.h>

ATTinyBME280 sensor(0x76);

void setup() {
    if (!sensor.begin(25)) {
      // Sensor not available; retry begin() later if required.
    }
}

void loop() {
    if (sensor.isConnected() && sensor.readData()) {
        // sensor.temperature: degrees Celsius
        // sensor.pressure: hectopascals
        // sensor.humidity: percent relative humidity
    }
}
```

`begin()` must return `true` before calling `readData()`. Use `isConnected()` to
check the BME280 chip ID (`0x60`) after initialization. If communication fails,
`readData()` returns `false` and the driver attempts a bounded bus recovery.

Example with error checking:

```cpp
ATTinyBME280 sensor(0x76);

if (sensor.begin(25)) {
    if (sensor.readData()) {
        // Measurement is valid.
    }
}
```

### Public API

- `ATTinyBME280(address)` creates a sensor object for the 7-bit I2C address.
- `begin(clock_khz)` initializes USI, verifies chip ID `0x60`, reads calibration
  data and configures forced mode with x1 oversampling.
- `isConnected()` checks the BME280 chip ID without reloading calibration data.
- `readData()` starts one forced measurement and updates `temperature`, `pressure`
  and `humidity`. It returns `false` on an invalid measurement or I2C failure.
- `prepareSleep()` releases the I2C lines before deep sleep.

The valid address range is `0x00` to `0x7F`; normal BME280 addresses are `0x76`
and `0x77`. Always check the return value of `begin()` before calling `readData()`.

## Power and sleep

Call `prepareSleep()` before putting the ATtiny into deep sleep. This releases SDA
and SCL as inputs with pull-ups enabled by the ATtiny port configuration.

## Timing and resource notes

- The default I2C clock is 25 kHz.
- The effective supported range is 1 to 100 kHz. A value of `0` is treated as
  1 kHz, and requested values above 100 kHz are limited to 100 kHz.
- The requested frequency is a target value. Loop and register overhead can make
  the measured bus frequency slightly lower, especially on slow ATtiny clocks.
- Measurements use forced mode with pressure, temperature and humidity oversampling
  set to x1.
- The conversion wait in `readData()` is fixed for this x1 configuration.
- The driver uses floating-point output values, which increases flash usage on AVR
  compared with an integer-only implementation.
- The implementation is intended for small ATtiny projects and does not use
  `Arduino.h`.

## Limitations

- SDA and SCL pins are not configurable through the public API.
- The library requires a microcontroller with USI registers.
- Bus recovery cannot fix a permanently powered-off or electrically defective
  device that holds SDA or SCL low.
- This library does not provide a built-in I2C bus multiplexer or power isolation.
- The implementation is currently written for the ATtiny25/45/85 USI register
  layout and is not a general-purpose AVR I2C library.

## Troubleshooting

- If both sensors are not detected, verify SDA, SCL, common GND, sensor supply and
  external pull-ups.
- If one sensor disappears when its VCC or GND is removed, disconnect its SDA/SCL
  lines as well or use proper bus/power isolation. A powered-off sensor can load
  the shared bus.
- If readings are incorrect, verify the selected MCU clock and `F_CPU` value.
- If upload fails, disconnect the BME280 from pins shared with the ISP interface
  while programming, then reconnect it for runtime testing.

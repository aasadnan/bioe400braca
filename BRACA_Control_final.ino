#include "programmable_air.h"
#include <Adafruit_NeoPixel.h>

#define DEBUG 1

int state = UN_KNOWN;

int sensorpin = A2;
int sensor;

// === Pressure Control Parameters ===
float targetPressure = 30.0;         // Desired pressure in mmHg
float pressureTolerance = 5.0;        // Acceptable error margin (mmHg)

// === Calibration constants for voltage-to-pressure conversion ===
const float A = 0.238;
const float B = 1.19;

void setup() {
  Serial.begin(9600);
  initializePins();
}

void loop() {
  // Read analog sensor and convert to voltage
  sensor = analogRead(sensorpin);
  float voltage = sensor * (5.0 / 1023.0);

  // Convert voltage to pressure using exponential model
  float pressure = A * exp(B * voltage);

  // Display pressure data
  showPressure();
  Serial.print("Sensor: "); Serial.print(sensor);
  Serial.print(" | Voltage: "); Serial.print(voltage, 3);
  Serial.print(" V | Pressure: "); Serial.print(pressure, 2);
  Serial.println(" mmHg");

  // Calculate pressure error
  float error = targetPressure - pressure;
  int power = (error / 10 ) * 100;
 //power = constrain(power, 0, 100);


  // === Feedback Control Logic ===
  if (abs(error) < pressureTolerance) {
    // Within acceptable range — vent
    if (state != VENTING) {
      switchOffPumps();
    }
  } else if (error > 0) {
    // Pressure too low — inflate
    if (state != BLOWING) {
      switchOnPump(2, power); // Inflate
      switchOffPump(1);
      blow();
      state = BLOWING;
    }
  } else {
    // Pressure too high — deflate
    if (state != SUCKING) {
      switchOnPump(1, power); // Deflate
      switchOffPump(2);
      suck();
      state = SUCKING;
    }
  }

  delayWhileReadingPressure(200); // Delay before next reading
}

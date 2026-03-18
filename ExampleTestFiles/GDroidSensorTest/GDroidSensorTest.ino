/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>

const int sensorNr = 3;

int xshutPins[] = {2, 10, 11};

VL53L0X sensors[sensorNr];

void setup()
{
  Serial.begin(19200);
  Wire.begin(6, 7);
  for (int i = 0; i < sensorNr; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }

  for (int i = 0; i < sensorNr; i++) {
    pinMode(xshutPins[i], INPUT);
    delay(10);

    sensors[i].setTimeout(500);
    if (!sensors[i].init())
    {
      Serial.println("Failed to detect and initialize sensor!");
      while (1) {}
    }
    sensors[i].setAddress(0x2A + i);
    sensors[i].startContinuous();
  }
}

void loop()
{
  for (int i = 0; i < sensorNr; i++) {
    Serial.print(sensors[i].readRangeContinuousMillimeters());
    Serial.print('\t');
    if (sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  }
  Serial.println();
}

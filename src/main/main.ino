
#include <Wire.h>
#include <VL53L0X.h>

#include <WiFi.h>

#define PIN_MOTLE_SPD 23
#define PIN_MOTLE_BAC 22
#define PIN_MOTLE_FOR 21
#define PIN_MOTRI_FOR 20
#define PIN_MOTRI_BAC 19
#define PIN_MOTRI_SPD 18

#define PIN_MOTRI_ENC 13
#define PIN_MOTLE_ENC 12

#define PIN_SENS_1 10
#define PIN_SENS_2 11
#define PIN_SENS_3 2

#define PIN_SDA 6
#define PIN_SCL 7

#define TURN_TRIGGER_DISTANCE 500 // distance to turn at in mm
#define REVERSE_TRIGGER_DISTANCE 100 // distance to reverse and turn at in mm
#define SLOW_TRIGGER_DISTANCE 500 // distance to slow down at in mm

#define SERIAL_BAUDS 19200 // serial baud for debugging


const int sensorNr = 3;
int distance[sensorNr];

int xshutPins[] = {PIN_SENS_1, PIN_SENS_2, PIN_SENS_3};

VL53L0X sensors[sensorNr];

void setup() {
  Serial.begin(SERIAL_BAUDS);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (initializeSensors() == 404) {
    Serial.println("Sensors failed");
    while (1) {}
  }
  // put your setup code here, to run once:
  initializeMotors();
  // WiFi.mode(WIFI_AP);
  // delay(10);
  // int channel = 1;
  // int ssid_hidden = 0;
  // int max_connection = 4;
  // WiFi.softAP("ESP-PAR", "passphrase", channel, ssid_hidden, max_connection, false, WIFI_AUTH_WPA3_PSK);
  delay(5000); // Start waiting for go!
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < sensorNr; i++) {
    distance[i] = sensors[i].readRangeContinuousMillimeters();
    if (sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  }
  driveLogic(distance);
}

int initializeSensors() {
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
      return 404;
    }
    sensors[i].setAddress(0x2A + i);
    sensors[i].startContinuous();
  }
  return 0;
}


int initializeMotEncoders() {
    pinMode(PIN_MOTLE_ENC, INPUT);
    pinMode(PIN_MOTRI_ENC, INPUT);
    return 0;
}

int initializeMotors() {
    pinMode(PIN_MOTLE_FOR, OUTPUT);
    pinMode(PIN_MOTLE_BAC, OUTPUT);
    pinMode(PIN_MOTRI_FOR, OUTPUT);
    pinMode(PIN_MOTRI_BAC, OUTPUT);

    pinMode(PIN_MOTLE_SPD, OUTPUT);
    pinMode(PIN_MOTRI_SPD, OUTPUT);
    return 0;
}

int motorRun(int motorSpeed, int MotorID) {
    if (motorSpeed == 0) {
        if (MotorID == 'L') {
            digitalWrite(PIN_MOTLE_FOR, LOW);
            digitalWrite(PIN_MOTLE_BAC, LOW);
            analogWrite(PIN_MOTLE_SPD, 0);
            return 0;
        }
        digitalWrite(PIN_MOTRI_FOR, LOW);
        digitalWrite(PIN_MOTRI_BAC, LOW);
        analogWrite(PIN_MOTRI_SPD, 0);
        return 0;
    }
    if (motorSpeed > 0) {
        if (MotorID == 'L') {
            digitalWrite(PIN_MOTLE_FOR, HIGH);
            digitalWrite(PIN_MOTLE_BAC, LOW);
            analogWrite(PIN_MOTLE_SPD, abs(motorSpeed));
            return 0;
        }
        digitalWrite(PIN_MOTRI_FOR, HIGH);
        digitalWrite(PIN_MOTRI_BAC, LOW);
        analogWrite(PIN_MOTRI_SPD, abs(motorSpeed));
    }
    if (motorSpeed < 0) {
        if (MotorID == 'L') {
            digitalWrite(PIN_MOTLE_FOR, LOW);
            digitalWrite(PIN_MOTLE_BAC, HIGH);
            analogWrite(PIN_MOTLE_SPD, motorSpeed);
            return 0;
        }
        digitalWrite(PIN_MOTRI_FOR, LOW);
        digitalWrite(PIN_MOTRI_BAC, HIGH);
        analogWrite(PIN_MOTRI_SPD, motorSpeed);
        return 0;
    }
    return 101;
}

void driveFancy(int speedLeft, int speedRight) {
    motorRun(speedLeft, 'L');
    motorRun(speedRight, 'R');
}

void driveForward(int speed) {
    motorRun(speed, 'L');
    motorRun(speed, 'R');
}

void driveBackward(int speed) {
    motorRun(-speed, 'L');
    motorRun(-speed, 'R');
}

void driveTurnLeft(int speed) {
    motorRun(speed, 'L');
    motorRun(-speed, 'R');
}

void driveTurnRight(int speed) {
    motorRun(-speed, 'L');
    motorRun(speed, 'R');
}

void driveStop() {
    motorRun(0, 'L');
    motorRun(0, 'R');
}

int reverseAtWall(int sensorFront) {
    if (sensorFront < REVERSE_TRIGGER_DISTANCE) {
        driveBackward(255);
        delay(700);
        return 3;
    } else if (sensorFront < SLOW_TRIGGER_DISTANCE) {
        driveForward(100);
        return 2;
    } else {
        driveForward(255);
        return 1;
    }
}

void turnAtCurve(int sensorLeft, int sensorFront, int sensorRight) {
    
    if (sensorLeft < TURN_TRIGGER_DISTANCE) {
        driveFancy(100, 200);
    } 
    if (sensorRight < TURN_TRIGGER_DISTANCE) {
        driveFancy(200, 100);
    }

}

void driveLogic(int sensorReadings[]) {
    reverseAtWall(sensorReadings[1]);
    turnAtCurve(sensorReadings[0], sensorReadings[1], sensorReadings[2]);
    
}
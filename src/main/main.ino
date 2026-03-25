
#include <Wire.h>
#include <VL53L0X.h>

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
#define REVERSE_TRIGGER_DISTANCE 120 // distance to reverse and turn at in mm
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
    ESP.restart();
  }
  // put your setup code here, to run once:
  initializeMotors();

  delay(5000); // Start waiting for go!
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < sensorNr; i++) {
    distance[i] = sensors[i].readRangeContinuousMillimeters();
    if (distance[i] == 65535) { 
        Serial.print("TIMEOUT"); 
        sensors[i].timeoutOccurred();
        safetyStop();
        ESP.restart();
        return;
    }
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

void safetyStop() {
    driveStop();
    Serial.println("We are stopping for our own safety and for the annoyance of others!");
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

void debugDuringDrive(int distance[]) {
    Serial.print("Sensor distances in order 1 | 2 | 3 : ");
    Serial.print(distance[0]);
    Serial.print("\t");
    Serial.print(distance[1]);
    Serial.print("\t");
    Serial.println(distance[2]);
}

void driveLogic(int sensorReadings[]) {
    debugDuringDrive(sensorReadings);
    reverseAtWall(sensorReadings[1]);
    turnAtCurve(sensorReadings[0], sensorReadings[1], sensorReadings[2]);
    
}

void reinitializeSensor(int i) {
  Serial.print("Reinitializing sensor ");
  Serial.println(i);

  // Shutdown sensor
  pinMode(xshutPins[i], OUTPUT);
  digitalWrite(xshutPins[i], LOW);
  delay(10);

  // Power it back on
  pinMode(xshutPins[i], INPUT);
  delay(10);

  // Re-init sensor
  sensors[i].setTimeout(500);
  if (!sensors[i].init()) {
    Serial.print("Failed to reinit sensor ");
    Serial.println(i);
    return;
  }

  sensors[i].setAddress(0x2A + i);
  sensors[i].startContinuous();

  Serial.print("Sensor ");
  Serial.print(i);
  Serial.println(" reinitialized");
}
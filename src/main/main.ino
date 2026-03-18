
#include <Wire.h>
#include <VL53L0X.h>

#include <WiFi.h>
#include <WebServer.h>

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

WebServer server(80);

String currentCommand = "";
int currentSpeed = 0;
const int defaultSpeed = 100;

const int sensorNr = 3;
int distance[sensorNr];

int xshutPins[] = {PIN_SENS_1, PIN_SENS_2, PIN_SENS_3};

VL53L0X sensors[sensorNr];

void setup() {
  Serial.begin(19200);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (initializeSensors() == 404) {
    Serial.println("Sensors failed");
    while (1) {}
  }
  // put your setup code here, to run once:
  initializeMotors();
  WiFi.mode(WIFI_AP);
  delay(10);
  int channel = 1;
  int ssid_hidden = 0;
  int max_connection = 4;
  WiFi.softAP("ESP-PAR", "passphrase", channel, ssid_hidden, max_connection, false, WIFI_AUTH_WPA3_PSK);

  server.onNotFound(handleCommand);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  for (int i = 0; i < sensorNr; i++) {
    distance[i] = sensors[i].readRangeContinuousMillimeters();
    if (sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  }
  driveLogic(distance);
  */
  server.handleClient();
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

int stopAtWall(int sensorFront) {
    if (sensorFront < 100) {
        driveStop();
        return 3;
    } else if (sensorFront < 300) {
        driveForward(100);
        return 2;
    } else {
        driveForward(255);
        return 1;
    }
}

void turnAtCurve(int sensorLeft,int sensorFront, int sensorRight) {
    
    if (sensorLeft < 250) {
        driveFancy(100, 200);
    } 
    if (sensorRight < 250) {
        driveFancy(200, 100);
    }

}

void driveLogic(int sensorReadings[]) {
    stopAtWall(sensorReadings[1]);
    turnAtCurve(sensorReadings[0], sensorReadings[1], sensorReadings[2]);
    
}

void handleCommand() {
  String request = server.uri();
  if (request.startsWith("/")) {
    request = request.substring(1);
  }
  
  // Parse command and speed (if provided)
  if (request.indexOf(":") != -1) {
    int colonIndex = request.indexOf(":");
    currentCommand = request.substring(0, colonIndex);
    String speedStr = request.substring(colonIndex + 1);
    currentSpeed = speedStr.toInt();
  } else {
    currentCommand = request;
    // Only assign default speed to movement commands (F, B, L, R)
    // S, X, Y, Z should have speed = 0
    if (currentCommand == "F" || currentCommand == "B" || 
        currentCommand == "L" || currentCommand == "R") {
      currentSpeed = defaultSpeed;
    } else {
      currentSpeed = 0; // For S, X, Y, Z and any other special commands
    }
  }
  
  // Process command and control robot
  controlRobot();
  displayCommand();
  
  // Send response back to client
  server.send(200, "text/plain", "Command received: " + request);
}

void controlRobot() {
    driveStop();
    if (currentCommand == "F") {
        driveForward(currentSpeed);
    }
    if (currentCommand == "B") {
        driveBackward(currentSpeed);
    }
    if (currentCommand == "L") {
        driveTurnLeft(currentSpeed);
    }
    if (currentCommand == "R") {
        driveTurnRight(currentSpeed);
    }
    if (currentCommand == "S") {
        driveStop();
    }
}

void displayCommand() {
  Serial.println("-------------------");
  Serial.print("Received Command: ");
  Serial.println(currentCommand);
  
  // Only display speed for commands that use it
  if (currentSpeed > 0 && (currentCommand == "F" || currentCommand == "B" || 
                           currentCommand == "L" || currentCommand == "R")) {
    Serial.print("Speed Value: ");
    Serial.println(currentSpeed);
  }
  
  if (currentCommand == "F") {
    Serial.println("Action: Moving Forward");
  } else if (currentCommand == "B") {
    Serial.println("Action: Moving Backward");
  } else if (currentCommand == "L") {
    Serial.println("Action: Turning Left");
  } else if (currentCommand == "R") {
    Serial.println("Action: Turning Right");
  } else if (currentCommand == "S") {
    Serial.println("Action: Stop");
  } else if (currentCommand == "X") {
    Serial.println("Action: Custom X");
  } else if (currentCommand == "Y") {
    Serial.println("Action: Custom Y");
  } else if (currentCommand == "Z") {
    Serial.println("Action: Custom Z");
  }
  Serial.println("-------------------");
}
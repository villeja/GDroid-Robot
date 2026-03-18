
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

void setup() {
  // put your setup code here, to run once:
  initializeMotors();
  Serial.begin(19200);
}

void loop() {
  // put your main code here, to run repeatedly:
  driveForward(100);
  Serial.println("FORWARD!!!");
  delay(5000);
  driveTurnRight(100);
  Serial.println("Lets go right..");
  delay(900);
  driveBackward(100);
  Serial.println("RETREAT!!! WE DONE GOOFED UP!!!");
  delay(5000);
  driveTurnLeft(100);
  Serial.println("I want to go left now.");
  delay(900);
  driveStop();
  Serial.println("Im bored, gonna wait until I want to do stuff again..");
  delay(10000);
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
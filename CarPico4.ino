#include <WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

const char* SSID = "CAR-AP";
const char* PASS = "stasis1234";
const uint16_t PORT = 8888;

WiFiUDP udp;
char packet[64];

const uint8_t LEFT_IN1  = 16;
const uint8_t LEFT_IN2  = 17;
const uint8_t RIGHT_IN1 = 18;
const uint8_t RIGHT_IN2 = 19;
const uint8_t SERVO_PIN = 14;

bool invertLeft = false;
bool invertRight = true;
int speedPWM = 0;

Servo turnServo;

const int SERVO_CENTER = 90;
const int SERVO_STEP = 10;
const unsigned long SERVO_RETURN_MS = 500;

bool servoBusy = false;
bool servoAttached = false;
int servoHomeAngle = SERVO_CENTER;
unsigned long servoMoveStart = 0;

void driveOne(int pwm, uint8_t in1, uint8_t in2) {
  pwm = constrain(pwm, -255, 255);

  if (pwm == 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    return;
  }

  if (pwm > 0) {
    analogWrite(in1, pwm);
    digitalWrite(in2, LOW);
  } else {
    digitalWrite(in1, LOW);
    analogWrite(in2, -pwm);
  }
}

void driveTank(int leftPwm, int rightPwm) {
  if (invertLeft) leftPwm = -leftPwm;
  if (invertRight) rightPwm = -rightPwm;
  driveOne(leftPwm, LEFT_IN1, LEFT_IN2);
  driveOne(rightPwm, RIGHT_IN1, RIGHT_IN2);
}

void stopAll() {
  driveOne(0, LEFT_IN1, LEFT_IN2);
  driveOne(0, RIGHT_IN1, RIGHT_IN2);
}

void attachServoIfNeeded() {
  if (!servoAttached) {
    Serial.print("[SERVO] attaching on pin ");
    Serial.println(SERVO_PIN);
    turnServo.attach(SERVO_PIN, 500, 2400);
    servoAttached = true;
    delay(20);
  }
}

void detachServoIfNeeded() {
  if (servoAttached) {
    Serial.println("[SERVO] detaching");
    turnServo.detach();
    servoAttached = false;
  }
}

void servoKick() {
  if (servoBusy) {
    Serial.println("[SERVO] trigger ignored, servo busy");
    return;
  }

  attachServoIfNeeded();

  servoHomeAngle = SERVO_CENTER;
  int target = servoHomeAngle + SERVO_STEP;
  target = constrain(target, 0, 180);

  Serial.print("[SERVO] one-shot move from ");
  Serial.print(servoHomeAngle);
  Serial.print(" to ");
  Serial.println(target);

  turnServo.write(target);
  servoMoveStart = millis();
  servoBusy = true;
}

void updateServo() {
  if (servoBusy && (millis() - servoMoveStart >= SERVO_RETURN_MS)) {
    Serial.print("[SERVO] returning to ");
    Serial.println(servoHomeAngle);
    turnServo.write(servoHomeAngle);
    delay(150);
    servoBusy = false;
    detachServoIfNeeded();
    Serial.println("[SERVO] one-shot complete");
  }
}

void handleCommand(const char* cmd) {
  Serial.print("[CMD] ");
  Serial.println(cmd);

  if (strcmp(cmd, "F") == 0) {
    Serial.println("[ACTION] forward");
    driveTank(speedPWM, speedPWM);
  } else if (strcmp(cmd, "B") == 0) {
    Serial.println("[ACTION] backward");
    driveTank(-speedPWM, -speedPWM);
  } else if (strcmp(cmd, "L") == 0) {
    Serial.println("[ACTION] left");
    driveTank(-speedPWM, speedPWM);
  } else if (strcmp(cmd, "R") == 0) {
    Serial.println("[ACTION] right");
    driveTank(speedPWM, -speedPWM);
  } else if (strcmp(cmd, "S") == 0) {
    Serial.println("[ACTION] servo one-shot");
    digitalWrite(LED_BUILTIN, HIGH);
    servoKick();
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
  } else if (strcmp(cmd, "X") == 0) {
    Serial.println("[ACTION] stop");
    stopAll();
  } else {
    Serial.println("[ACTION] unknown");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("[SETUP] booting");

  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  stopAll();

  servoAttached = false;
  servoBusy = false;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);
  udp.begin(PORT);

  Serial.println("[SETUP] Pico ready");
  Serial.print("[SETUP] AP IP = ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  updateServo();

  int n = udp.parsePacket();
  if (n > 0) {
    int len = udp.read(packet, sizeof(packet) - 1);
    if (len > 0) {
      packet[len] = 0;
      handleCommand(packet);
    }
  }
}
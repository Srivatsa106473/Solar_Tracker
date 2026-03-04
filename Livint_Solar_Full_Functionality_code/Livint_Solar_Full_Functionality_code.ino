#include <Arduino.h>

// ===================================
// BTS7960 Motor Driver Pins
// ===================================
const int RPWM = 25;   // Motor clockwise
const int LPWM = 26;   // Motor counter-clockwise
const int R_EN = 12;
const int L_EN = 14;

// ===================================
// Photodiode Pins
// ===================================
const int leftDiode   = 32;
const int centerDiode = 35;
const int rightDiode  = 34;

// ===================================
// PWM & Control Settings
// ===================================
const int PWM_MAX     = 4095;
const int MOTOR_MAX   = 3000;
const int MOTOR_MIN   = 800;

// ===================================
// Sensor Thresholds (TUNE THESE)
// ===================================
const int deadzone     = 20;     // L-R tolerance
const int CENTER_LOCK  = 3000;   // Center diode vertical lock
const int MAX_DIFF     = 1200;   // L-R diff when far from vertical

// ===================================
void stopMotor() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

// ===================================
void setup() {
  Serial.begin(115200);
  delay(300);

  analogReadResolution(12);

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  stopMotor();

  Serial.println("\n=== ESP32 SOLAR TRACKER READY ===");
}

// ===================================
void loop() {

  int L = analogRead(leftDiode);
  int C = analogRead(centerDiode);
  int R = analogRead(rightDiode);

  int diff = L - R;

  Serial.print("L="); Serial.print(L);
  Serial.print("  C="); Serial.print(C);
  Serial.print("  R="); Serial.print(R);
  Serial.print("  DIFF="); Serial.println(diff);

  // ===================================
  // CENTER LOCK → STOP AT VERTICAL
  // ===================================
  if (C > CENTER_LOCK) {
    Serial.println("CENTER HIGH → VERTICAL POSITION → STOP");
    stopMotor();
    delay(300);
    return;
  }

  // ===================================
  //  DEADZONE → STOP
  // ===================================
  if (abs(diff) < deadzone) {
    Serial.println("BALANCED → STOP");
    stopMotor();
    delay(300);
    return;
  }

  // ===================================
  //  SPEED CONTROL (SLOW NEAR TARGET)
  // ===================================
  int speed = map(abs(diff), deadzone, MAX_DIFF, MOTOR_MIN, MOTOR_MAX);
  speed = constrain(speed, MOTOR_MIN, MOTOR_MAX);

  // ===================================
  //  DIRECTION CONTROL
  // ===================================
  if (diff > 0) {
    Serial.println("ROTATE ANTI-CLOCKWISE");
    analogWrite(LPWM, speed);
    analogWrite(RPWM, 0);
  } 
  else {
    Serial.println("ROTATE CLOCKWISE");
    analogWrite(RPWM, speed);
    analogWrite(LPWM, 0);
  }

  delay(150);
}

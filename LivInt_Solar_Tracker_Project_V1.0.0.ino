// This code is designed for ESP32 and uses its Deep Sleep feature for power efficiency.
// It tracks the sun every 5 minutes during the day and returns to a safe position in high winds.

// ===================================
// === Stepper Motor Control Pins ===
// ===================================
const int PULSE_PIN_1 = 2;    // PUL+ on DM556
const int PULSE_PIN_2 = 5;    // PUL+ on DM556
const int DIR_PIN_1 = 13;     // DIR+ on DM556
const int DIR_PIN_2 = 18;     // DIR+ on DM556
const int ENABLE_PIN = 4;     // ENA+ on DM556

// ===================================
// === Photodiode Analog Input Pins ===
// ===================================
const int leftDiode = 34;     // Left sensor
const int centerDiode = 35;   // Center sensor
const int rightDiode = 32;    // Right sensor
const int NIGHT_THRESHOLD = 200; // ADJUST: Analog value below this is considered 'night' (0-4095)

// ===================================
// === Wind Sensor (Anemometer) Pins ===
// ===================================
// NOTE: Use a suitable analog pin. Pin 39 is common for ESP32 ADC1.
const int ANEMOMETER_PIN = 39; 

// ===================================
// === Motor & Tracking Settings ===
// ===================================
const int STEP_DELAY = 500;            // Microseconds between steps (pulse length)
const long MOVE_STEPS = 10;            // Steps to move each time an imbalance is detected
const float WIND_CUTOFF_MPS = 10.0;    // ADJUST: Wind speed threshold in m/s (e.g., 10 m/s for safety)
const bool CLOCKWISE = HIGH;           // Direction logic (HIGH or LOW)
const bool COUNTER_CLOCKWISE = LOW;

// === Timing Constants ===
const uint64_t DAY_SLEEP_TIME_US   = 5 * 60 * 1000000;  // 5 minutes in microseconds
const uint64_t NIGHT_SLEEP_TIME_US = 11 * 60 * 60 * 1000000; // 11 hours in microseconds

// === Anemometer Constants ===
// Adjust these based on your specific wind sensor's datasheet
const float minVoltage = 0.054;  
const float maxVoltage = 5.0;    
const float maxWindSpeed = 32.4; 

// === Global State Variables (Stored in RTC Memory to survive Deep Sleep) ===
// RTC_DATA_ATTR is essential for retaining values across wake-ups
RTC_DATA_ATTR volatile long currentPosition = 0; // Tracks the total steps moved from the zero position
RTC_DATA_ATTR volatile bool motorStopped = true; 


// ====================================================================
// === CORE LOGIC (Executed upon every wake-up) ===
// ====================================================================

void setup() {
  // Initialize motor control pins
  pinMode(PULSE_PIN_1, OUTPUT);
  pinMode(PULSE_PIN_2, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable driver (LOW is usually ENABLE)

  Serial.begin(115200);
  delay(100); // Give serial time to start
  Serial.println("Setup started....");
  
  Serial.print("\n-- Woke Up -- Position: ");
  Serial.print(currentPosition);
  Serial.print(", Time Passed: ");
  Serial.println(esp_timer_get_time() / 1000000); // Print how long it was asleep (in seconds)

  // 1. Check Wind Safety (ALWAYS run first upon wake-up)
  float windSpeed_mps = readWindSpeed();
  Serial.print("Wind Speed: ");
  Serial.print(windSpeed_mps);
  Serial.println(" m/s");
  
  if (windSpeed_mps > WIND_CUTOFF_MPS) {
    Serial.println("!!! HIGH WIND DETECTED - RETURNING TO SAFE POSITION !!!");
    returnToZero(); // Blocking move back to the starting position
    delay(500); // Wait for motor to settle
  } 

  // 2. Check Light Level (Sun/Night) and Perform Action
  if (isNight()) {
    Serial.println("It is night. Entering long sleep (11 hours).");
    esp_sleep_enable_timer_wakeup(NIGHT_SLEEP_TIME_US); 
  } else {
    // Day time tracking logic
    performTrackingLogic();

    Serial.println("Daytime check complete. Entering short sleep (5 minutes).");
    esp_sleep_enable_timer_wakeup(DAY_SLEEP_TIME_US);
  }

  // 3. Enter Deep Sleep
  Serial.flush(); // Wait for Serial to finish sending data
  esp_deep_sleep_start();
}

// loop() is not used because the device goes to sleep immediately after setup()
void loop() {
  // Empty
}

// ====================================================================
// === AUXILIARY FUNCTIONS ===
// ====================================================================

bool isNight() {
    // Read the center sensor as the primary light check
    int centerVal = analogRead(centerDiode);
    
    Serial.print("Center Light Level: ");
    Serial.println(centerVal);
    
    // If center is below the threshold, it is considered night
    return centerVal < NIGHT_THRESHOLD;
}

void performTrackingLogic() {
  // 1. Read Photodiodes
  int leftVal = analogRead(leftDiode);
  int centerVal = analogRead(centerDiode);
  int rightVal = analogRead(rightDiode);

  // Print debug values
  Serial.print("L: "); Serial.print(leftVal);
  Serial.print(" | C: "); Serial.print(centerVal);
  Serial.print(" | R: "); Serial.println(rightVal);

  // 2. Tracking Logic
  if (centerVal > leftVal && centerVal > rightVal) {
    // Center diode highest - stop motor
    stopMotor();
    Serial.println("CENTER MAX - MOTOR STOPPED");
  } else if (leftVal > centerVal && leftVal > rightVal) {
    // Left diode highest - rotate COUNTER_CLOCKWISE
    Serial.println("LEFT MAX - ROTATING CCW");
    rotateMotor(COUNTER_CLOCKWISE, COUNTER_CLOCKWISE, MOVE_STEPS); 
  } else if (rightVal > centerVal && rightVal > leftVal) {
    // Right diode highest - rotate CLOCKWISE
    Serial.println("RIGHT MAX - ROTATING CW");
    rotateMotor(CLOCKWISE, CLOCKWISE, MOVE_STEPS); 
  }
}

float readWindSpeed() {
  int adcValue = analogRead(ANEMOMETER_PIN);
  float voltage = (adcValue / 4095.0) * 3.3; // ESP32 ADC is typically 12-bit (4095) max and 3.3V

  if (voltage < minVoltage) {
    voltage = minVoltage;
  } else if (voltage > maxVoltage) {
    voltage = maxVoltage;
  }

  // Map the voltage to wind speed
  float windSpeed_mps = ((voltage - minVoltage) / (maxVoltage - minVoltage)) * maxWindSpeed;
  return windSpeed_mps;
}

void rotateMotor(bool dir1, bool dir2, long steps) {
  // Blocking function to move motor
  if (!motorStopped) return;
  motorStopped = false; 

  digitalWrite(DIR_PIN_1, dir1);
  digitalWrite(DIR_PIN_2, dir2);

  for (long i = 0; i < steps; i++) {
    digitalWrite(PULSE_PIN_1, HIGH);
    digitalWrite(PULSE_PIN_2, HIGH);

    delayMicroseconds(STEP_DELAY / 2);

    digitalWrite(PULSE_PIN_1, LOW);
    digitalWrite(PULSE_PIN_2, LOW);
    delayMicroseconds(STEP_DELAY / 2);

    // Update position tracker
    if (dir1 == CLOCKWISE) {
      currentPosition++;
    } else {
      currentPosition--;
    }
  }

  motorStopped = true; 
}

void stopMotor() {
  digitalWrite(PULSE_PIN_1, LOW);
  digitalWrite(PULSE_PIN_2, LOW);
  motorStopped = true;
}

void returnToZero() {
  if (currentPosition == 0) {
    Serial.println("Already at zero position.");
    return;
  }

  Serial.print("Returning to zero from steps: ");
  Serial.println(currentPosition);

  // Determine the direction to return to zero
  bool returnDir = (currentPosition > 0) ? COUNTER_CLOCKWISE : CLOCKWISE;

  // Set the direction pins
  digitalWrite(DIR_PIN_1, returnDir);
  digitalWrite(DIR_PIN_2, returnDir);

  // Calculate the absolute number of steps to return
  long stepsToMove = abs(currentPosition);

  for (long i = 0; i < stepsToMove; i++) {
    digitalWrite(PULSE_PIN_1, HIGH);
    digitalWrite(PULSE_PIN_2, HIGH);
    delayMicroseconds(STEP_DELAY / 2);
    digitalWrite(PULSE_PIN_1, LOW);
    digitalWrite(PULSE_PIN_2, LOW);
    delayMicroseconds(STEP_DELAY / 2);
  }
  
  // Reset the position tracker after movement is complete
  currentPosition = 0;
  Serial.println("Returned to zero. Position reset.");
}
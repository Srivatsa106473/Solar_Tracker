###🌞 ESP32 Solar Tracker with Wind Safety & Deep Sleep

This project implements a solar reflector/sun tracker system using the ESP32 microcontroller, designed to maximize energy efficiency and protect hardware in harsh conditions. The firmware leverages deep sleep mode to conserve power, periodically waking up to track the sun’s position using photodiodes and controlling stepper motors via DM556 drivers.

###✨ Features

##Efficient Sun Tracking

Uses three photodiodes (left, center, right) to detect light intensity.

Adjusts reflector position every 5 minutes during the day for optimal alignment.

##Deep Sleep Power Management

ESP32 sleeps for 5 minutes between daytime adjustments.

At night, the system sleeps for 11 hours, minimizing energy usage.

##Wind Safety Mechanism

Reads real-time wind speed from an anemometer.

If wind speed exceeds the defined cutoff (e.g., 10 m/s), the tracker returns safely to the zero position to prevent structural damage.

##Stepper Motor Control

Dual motors controlled with DM556 drivers.

Adjustable step size, direction, and speed with STEP_DELAY and MOVE_STEPS.

Position tracking retained across deep sleep cycles with RTC memory.

##Scalable & Customizable

Thresholds (light, wind, step size, timing) can be easily tuned for your environment.

Well-documented code for quick modifications and debugging.

###⚙️ Hardware Requirements

ESP32 development board

2 × NEMA stepper motors + DM556 drivers

3 × Photodiodes (LDRs or similar)

Anemometer (wind speed sensor, analog output)

Power supply capable of handling motor driver and ESP32

###🛠️ Code Highlights

Sun tracking logic using comparative light values from left, center, right sensors.

Wind speed calculation based on ADC values mapped to voltage and sensor specs.

Motor safety & return-to-zero function to reset system when needed.

RTC memory variables retain motor position across deep sleep cycles.

###🚀 Usage

Clone the repository:

git clone https://github.com/your-username/esp32-solar-tracker.git
cd esp32-solar-tracker


Open the .ino file in Arduino IDE or PlatformIO.

Adjust pin numbers and thresholds in the config section according to your setup.

Upload to ESP32 and connect hardware.

Monitor the system via Serial Monitor (115200 baud).

###📝 Example Behavior

Daytime → ESP32 wakes up every 5 min, checks light sensors, adjusts motors if needed.

High wind detected → Motors return to zero (safe position), system halts movement.

Nighttime → ESP32 sleeps for 11 hours until sunrise.

###📌 Applications

Solar reflectors

Solar panel alignment systems

Automated heliostats

Renewable energy projects

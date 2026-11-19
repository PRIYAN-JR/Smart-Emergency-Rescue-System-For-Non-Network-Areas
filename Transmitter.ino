#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <string.h>

// ---------------- GPS ----------------
// GPS TX -> Nano D4
SoftwareSerial gpsSerial(4, 3);
TinyGPSPlus gps;

// ---------------- RF -----------------
// RF transmitter DATA -> Nano D12
RH_ASK driver(2000, 11, 12, 10, false);

// ---------------- SOS BUTTON ----------
// Button between D2 and GND
const int SOS_BUTTON = 2;
bool emergencyMode = false;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ---------------- MAX30102 -------------
MAX30105 particleSensor;
bool sensorReady = false;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
byte validReadings = 0;

unsigned long lastBeatTime = 0;
unsigned long lastValidHRTime = 0;

float currentBPM = 0;
float averageBPM = 0;
int heartRate = 0;

// ---------------- GPS DATA -------------
double lastLat = 0.0;
double lastLon = 0.0;
bool haveLocation = false;

// ---------------- TRANSMISSION --------
unsigned long lastSend = 0;
const unsigned long sendInterval = 2000;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  pinMode(SOS_BUTTON, INPUT_PULLUP);

  Serial.println(F("GPS + RF + SOS + Heart Rate"));

  if (!driver.init()) {
    Serial.println(F("RF initialization FAILED"));
    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("RF transmitter ready"));

  Wire.begin();

  if (particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    // Settings from the working standalone test
    particleSensor.setup(60, 4, 2, 100, 411, 4096);
    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeIR(0x1F);

    sensorReady = true;
    Serial.println(F("MAX30102 ready"));
  } else {
    sensorReady = false;
    Serial.println(F("MAX30102 not detected"));
  }
}

// ---------------- HEART RATE ------------
// Call this frequently; do not add long delays here.
void updateHeartRate() {
  if (!sensorReady) {
    return;
  }

  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    // No finger: clear readings from the previous measurement
    validReadings = 0;
    rateSpot = 0;
    lastBeatTime = 0;
    averageBPM = 0;
    heartRate = 0;
    return;
  }

  if (checkForBeat(irValue)) {
    unsigned long now = millis();

    if (lastBeatTime != 0) {
      unsigned long delta = now - lastBeatTime;

      if (delta > 0) {
        float bpm = 60000.0 / delta;

        if (bpm >= 40 && bpm <= 200) {
          currentBPM = bpm;

          rates[rateSpot] = (byte)(bpm + 0.5);
          rateSpot = (rateSpot + 1) % RATE_SIZE;

          if (validReadings < RATE_SIZE) {
            validReadings++;
          }

          unsigned int sum = 0;

          for (byte i = 0; i < validReadings; i++) {
            sum += rates[i];
          }

          averageBPM = (float)sum / validReadings;

          // Require at least two valid intervals
          if (validReadings >= 2) {
            heartRate = (int)(averageBPM + 0.5);
            lastValidHRTime = now;

            Serial.print(F("Current BPM: "));
            Serial.print(currentBPM, 1);
            Serial.print(F(" | Average BPM: "));
            Serial.println(heartRate);
          }
        }
      }
    }

    lastBeatTime = now;
  }
}

// ---------------- MAIN LOOP --------------
void loop() {
  // 1. Read GPS continuously
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated() &&
      gps.location.isValid()) {
    lastLat = gps.location.lat();
    lastLon = gps.location.lng();
    haveLocation = true;
  }

  // 2. Check SOS button with debounce
  bool reading = digitalRead(SOS_BUTTON);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime >= debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // One press activates emergency mode.
      // It remains active until the Nano is reset or powered off.
      if (stableButtonState == LOW && !emergencyMode) {
        emergencyMode = true;
        Serial.println(F("SOS EMERGENCY ACTIVATED"));
      }
    }
  }

  lastButtonReading = reading;

  // 3. Update heart rate
  updateHeartRate();

  // 4. Send emergency data every 2 seconds
  if (emergencyMode &&
      millis() - lastSend >= sendInterval) {

    lastSend = millis();

    char message[RH_ASK_MAX_MESSAGE_LEN];
    char latText[16];
    char lonText[16];
    char hrText[5];

    // Use only a recent heart-rate estimate
    if (sensorReady &&
        heartRate > 0 &&
        millis() - lastValidHRTime <= 10000) {
      snprintf(hrText, sizeof(hrText), "%d", heartRate);
    } else {
      strcpy(hrText, "NA");
    }

    if (haveLocation) {
      dtostrf(lastLat, 0, 6, latText);
      dtostrf(lastLon, 0, 6, lonText);

      snprintf(message, sizeof(message),
               "SOS:1,LAT:%s,LON:%s,HR:%s",
               latText, lonText, hrText);
    } else {
      snprintf(message, sizeof(message),
               "SOS:1,GPS:NO FIX,HR:%s", hrText);
    }

    driver.send((uint8_t *)message, strlen(message));
    driver.waitPacketSent();

    Serial.print(F("Emergency packet sent: "));
    Serial.println(message);
  }
}

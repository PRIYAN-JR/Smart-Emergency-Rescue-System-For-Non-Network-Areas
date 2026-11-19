#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <string.h>
#include <stdio.h>

// OLED: SSD1306 128x64, low-memory page mode
U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(
  U8G2_R0, U8X8_PIN_NONE
);

// RF receiver DATA -> D11
RH_ASK driver(2000, 11, 12, 10, false);

// Buzzer positive -> D6, negative -> GND
const int BUZZER_PIN = 6;

unsigned long lastBeepTime = 0;
const unsigned long beepInterval = 2000;

void playBeepBeep() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 1500);
    delay(120);
    noTone(BUZZER_PIN);
    delay(100);
  }

  delay(150);

  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 1000);
    delay(180);
    noTone(BUZZER_PIN);
    delay(100);
  }
}

void showData(const char *lat,
              const char *lon,
              const char *hr,
              bool sos) {
  display.firstPage();
  do {
    display.setFont(u8g2_font_6x10_tf);

    display.drawStr(0, 12, sos ? "!!! SOS ALERT !!!" : "GPS RESCUE SYSTEM");
    display.drawStr(0, 27, lat);
    display.drawStr(0, 40, lon);
    display.drawStr(0, 53, hr);
  } while (display.nextPage());
}

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  display.begin();

  display.firstPage();
  do {
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 30, "Waiting for data...");
  } while (display.nextPage());

  if (!driver.init()) {
    display.firstPage();
    do {
      display.setFont(u8g2_font_6x10_tf);
      display.drawStr(0, 30, "RF FAILED");
    } while (display.nextPage());

    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("RF receiver ready"));
}

void loop() {
  uint8_t buffer[RH_ASK_MAX_MESSAGE_LEN + 1];
  uint8_t length = RH_ASK_MAX_MESSAGE_LEN;

  if (driver.recv(buffer, &length)) {
    if (length > RH_ASK_MAX_MESSAGE_LEN) {
      length = RH_ASK_MAX_MESSAGE_LEN;
    }

    buffer[length] = '\0';

    Serial.print(F("Received: "));
    Serial.println((char *)buffer);

    // Example packet:
    // SOS:1,LAT:11.056714,LON:78.048195,HR:76

    char packet[RH_ASK_MAX_MESSAGE_LEN + 1];
    strncpy(packet, (char *)buffer, sizeof(packet) - 1);
    packet[sizeof(packet) - 1] = '\0';

    char lat[22] = "Latitude: NO FIX";
    char lon[22] = "Longitude: NO FIX";
    char hr[16] = "Heart rate: N/A";

    bool sos = false;
    bool haveLat = false;
    bool haveLon = false;

    char *savePtr;
    char *field = strtok_r(packet, ",", &savePtr);

    while (field != NULL) {
      if (strcmp(field, "SOS:1") == 0) {
        sos = true;
      } else if (strncmp(field, "LAT:", 4) == 0) {
        snprintf(lat, sizeof(lat), "Lat: %s", field + 4);
        haveLat = true;
      } else if (strncmp(field, "LON:", 4) == 0) {
        snprintf(lon, sizeof(lon), "Lon: %s", field + 4);
        haveLon = true;
      } else if (strncmp(field, "HR:", 3) == 0) {
        if (strcmp(field + 3, "NA") == 0) {
          strcpy(hr, "Heart rate: N/A");
        } else {
          snprintf(hr, sizeof(hr), "Heart rate: %s BPM",
                   field + 3);
        }
      } else if (strcmp(field, "GPS:NO FIX") == 0) {
        strcpy(lat, "Latitude: NO FIX");
        strcpy(lon, "Longitude: NO FIX");
      }

      field = strtok_r(NULL, ",", &savePtr);
    }

    if (!haveLat || !haveLon) {
      strcpy(lat, "Latitude: NO FIX");
      strcpy(lon, "Longitude: NO FIX");
    }

    showData(lat, lon, hr, sos);

    // Beep for SOS packets, at most once per interval.
    if (sos && millis() - lastBeepTime >= beepInterval) {
      lastBeepTime = millis();
      playBeepBeep();
    }
  }
}
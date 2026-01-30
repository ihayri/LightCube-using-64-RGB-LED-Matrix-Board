#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "WS_QMI8658.h"
#include "WS_Matrix.h"

// Sleep/Wake Configuration
#define FACE_DOWN_SLEEP_TIME 3000    // 3 seconds face down = sleep
#define WAKE_PIN GPIO_NUM_4          // GPIO4 connected to TTP223 module output

// TWO SEPARATE DEFINES TO INDIVIDUALLY CONTROL BOTH FUNCTIONS
#define ENABLE_WHITE_DIAL 1    // 1 = White dial ON, 0 = White dial OFF
#define ENABLE_COLOR_FLIP 1    // 1 = Color flip ON, 0 = Color flip OFF

extern Adafruit_NeoPixel pixels; 
extern SensorQMI8658 QMI;
extern IMUdata Accel;
extern IMUdata Gyro;

// Sleep control variables
unsigned long faceDownStartTime = 0;
bool isFaceDown = false;

// Existing state variables
float currentHeading = 127.0; 
int currentPreset = 0;
int stableSide = -1;
int lastProcessedSide = -1;
unsigned long sideEntryTime = 0;
uint16_t rainbowHue = 0; 

// --- 14 PRESETS ---
uint32_t presets[] = {
  0xFF0000, 0xFF4000, 0xFFD000, 0xADFF2F, 0x00FF00, 
  0x00FF80, 0x00FFFF, 0x007FFF, 0x0000FF, 0x8B00FF, 
  0xFF00FF, 0xFF007F, 
  0x000000, // 12: Rainbow
  0x000000  // 13: Police
};
const int totalPresets = 14;

void updateMatrix(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 64; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  
  // First: Disable all internal pull-ups/pull-downs on wake pin
  gpio_pullup_dis((gpio_num_t)WAKE_PIN);
  gpio_pulldown_dis((gpio_num_t)WAKE_PIN);
  
  // Then set as input
  pinMode(WAKE_PIN, INPUT);
  
  // Initialize hardware
  Matrix_Init(); 
  QMI8658_Init(); 
  pixels.setBrightness(70); 
  
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    // Just woke from sleep
    updateMatrix(255, 0, 0); // Show red immediately
  } else {
    updateMatrix(0, 0, 0); // Start with LEDs off
  }
}

void checkFaceDownSleep() {
  if (stableSide == 1) {
    if (!isFaceDown) {
      faceDownStartTime = millis();
      isFaceDown = true;
    } else if (millis() - faceDownStartTime >= FACE_DOWN_SLEEP_TIME) {
      enterDeepSleep();
    }
  } else if (isFaceDown) {
    isFaceDown = false;
    faceDownStartTime = 0;
  }
}

void enterDeepSleep() {
  // Critical: Prepare ALL GPIOs for sleep
  
  // 1. Turn off NeoPixels
  pixels.clear();
  pixels.show();
  
  // 2. Set NeoPixel data pin to INPUT (prevents leakage)
  // Change 14 to your actual NeoPixel pin if different
  gpio_pullup_dis(GPIO_NUM_14);
  gpio_pulldown_dis(GPIO_NUM_14);
  pinMode(14, INPUT);
  
  // 3. Disable pull-ups/pull-downs on wake pin (already done in setup)
  
  // 4. Read current TTP223 state to debug
  int ttpState = digitalRead(WAKE_PIN);
  
  // 5. Configure wakeup - MOST TTP223 modules are HIGH when touched
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void loop() {
  QMI8658_Loop();
  
  int detectedSide = -1;
  float limit = 0.90;

  if (Accel.z < -limit)      detectedSide = 0;
  else if (Accel.z > limit)  detectedSide = 1;
  else if (Accel.x > limit)  detectedSide = 2;
  else if (Accel.x < -limit) detectedSide = 3;
  else if (Accel.y > limit)  detectedSide = 4;
  else if (Accel.y < -limit) detectedSide = 5;

  #if ENABLE_COLOR_FLIP
    if (detectedSide != -1 && detectedSide != stableSide) {
      if (sideEntryTime == 0) sideEntryTime = millis();
      if (millis() - sideEntryTime > 150) { 
        if (detectedSide >= 2 && stableSide >= 2) {
          int step = getRotationStepSecondCode(stableSide, detectedSide);
          currentPreset += step;
          if (currentPreset < 0) currentPreset = totalPresets - 1;
          if (currentPreset >= totalPresets) currentPreset = 0;
        }
        stableSide = detectedSide;
        sideEntryTime = 0;
      }
    } else { sideEntryTime = 0; }
  #endif

  checkFaceDownSleep();

  if (stableSide == 0) {
    #if ENABLE_WHITE_DIAL
      handleWhiteDial();
    #else
      #if ENABLE_COLOR_FLIP
        handleColorFlipSide0();
      #endif
    #endif
  } 
  else if (stableSide == 1) {
    updateMatrix(0, 0, 0);
  } 
  else if (stableSide >= 2) {
    #if ENABLE_COLOR_FLIP
      handleColorFlipSides2to5();
    #endif
  }
  
  delay(10);
}

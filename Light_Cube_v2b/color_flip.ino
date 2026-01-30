// --- SECOND CODE'S COLOR FLIP FUNCTIONS ---

void runRainbow() {
  rainbowHue += 256; 
  for (int i = 0; i < 64; i++) {
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(rainbowHue + (i * 64))));
  }
  pixels.show();
}

// --- SECOND CODE'S POLICE LOGIC ---
void runPolice() {
  static unsigned long nextEventTime = 0;
  static int patternStep = 0;
  int timings[] = {85, 70, 85, 320, 90, 70, 90, 420};
  if (millis() >= nextEventTime) {
    patternStep = (patternStep + 1) % 8;
    switch(patternStep) {
      case 0: case 2: updateMatrix(0, 0, 255); break; 
      case 4: case 6: updateMatrix(255, 0, 0); break; 
      default:        updateMatrix(0, 0, 0);   break; 
    }
    nextEventTime = millis() + timings[patternStep];
  }
}

// --- SECOND CODE'S ROTATION STEP FUNCTION ---
int getRotationStepSecondCode(int last, int current) {
  if (last == 2 && current == 5) return 1;
  if (last == 5 && current == 3) return 1;
  if (last == 3 && current == 4) return 1;
  if (last == 4 && current == 2) return 1;
  if (last == 2 && current == 4) return -1;
  if (last == 4 && current == 3) return -1;
  if (last == 3 && current == 5) return -1;
  if (last == 5 && current == 2) return -1;
  return 0;
}

void handleColorFlipSide0() {
  if (currentPreset == 12)      runRainbow();
  else if (currentPreset == 13) runPolice();
  else {
    uint32_t c = presets[currentPreset];
    updateMatrix((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
  }
}

void handleColorFlipSides2to5() {
  if (currentPreset == 12)      runRainbow();
  else if (currentPreset == 13) runPolice();
  else {
    uint32_t c = presets[currentPreset];
    updateMatrix((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
  }
}

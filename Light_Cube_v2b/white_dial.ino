void handleWhiteDial() {
  // ORIGINAL FIRST CODE WHITE DIAL (with correct sensitivity)
  if (abs(Gyro.z) > 2.5) { currentHeading += (Gyro.z * 0.05); }
  currentHeading = constrain(currentHeading, 0, 255);
  int dialPos = (int)currentHeading;
  int r, g, b;
  if (dialPos < 128) {
    r = 255; g = map(dialPos, 0, 127, 130, 240); b = map(dialPos, 0, 127, 20, 220);
  } else {
    r = map(dialPos, 128, 255, 255, 100); g = map(dialPos, 128, 255, 240, 210); b = 255;
  }
  updateMatrix(r, g, b);
}

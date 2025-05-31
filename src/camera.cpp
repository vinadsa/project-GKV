#include "camera.h"
#include "globals.h"

// Define global variables from globals.h that are primarily camera-related
float cameraAngleX = -90.0f;  // Changed to look towards X direction (rotate horizontally)
float cameraAngleY = -10.0f;  // Changed to look more forward/down instead of up
float cameraDistance = 15.0f;
float cameraTargetYOffset = 0.5f;

// If you add specific camera functions, implement them here.
// void updateCamera() { /* ... */ }
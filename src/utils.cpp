#include "utils.h"
#include "globals.h" 
#include <cmath>     

float degToRad(float deg) {
    return deg * (float)M_PI / 180.0f;
}

float clamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}
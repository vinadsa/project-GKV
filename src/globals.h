#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glut.h>
#include <vector>

// Define PI manually for reliability if M_PI is not available
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Global Constants ---
// Arena constants
const int GRID_SIZE = 150;          // Definisikan GRID_SIZE di sini
const float BOUNDS = 15.0f;
const float minGroundHeight = -10.0f;
const float defaultFallingHeight = -8.0f;
const float pathBaseHeight = 0.5f;

// Physics constants
const float gravity = 9.8f;
const float friction = 0.95f;
const float deltaTime = 0.016f;
const float inputPushForce = 40.0f;

// Mouse sensitivity
const float mouseSensitivity = 0.4f;

// Checkpoint radius
const float checkpointRadius = 1.5f;


// --- Extern Global Variables ---
// Marble position and velocity
extern float marbleX, marbleZ;
extern float marbleVX, marbleVZ;

// Camera Variables
extern float cameraAngleX;
extern float cameraAngleY;
extern float cameraDistance;
extern float cameraTargetYOffset;

// Mouse State Variables
extern bool isDragging;
extern int lastMouseX, lastMouseY;

// Keyboard State Variables
extern bool keyStates[256];

// Arena Geometry Data (Heightmap)
// Deklarasi ini sekarang valid karena GRID_SIZE adalah konstanta yang diketahui
extern float arenaHeights[GRID_SIZE][GRID_SIZE];

// Checkpoint System
struct Vec3 {
    float x, y, z;
};

extern std::vector<Vec3> checkpoints;
extern int activeCheckpointIndex;

#endif // GLOBALS_H
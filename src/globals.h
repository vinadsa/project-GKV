#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glut.h>
#include <vector>

// Forward declare GLUquadric to avoid including full glu.h here
struct GLUquadric;

// Define PI manually for reliability if M_PI is not available
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Global Constants ---
// Arena constants
const int GRID_SIZE = 150;          // Definisikan GRID_SIZE di sini
const float BOUNDS = 100.0f;
const float minGroundHeight = -20.0f; // Threshold for falling out of bounds
const float defaultFallingHeight = -8.0f;
const float pathBaseHeight = 0.5f;

// Physics constants
const float gravity = 25.0f;    // Adjusted gravity
const float friction = 0.98f;   // Damping factor
const float deltaTime = 0.016f; // Example fixed delta time
const float inputPushForce = 30.0f;

// Mouse sensitivity
const float mouseSensitivity = 0.4f;

// Checkpoint radius
const float checkpointRadius = 1.5f;

// Marble radius
const float marbleRadius = 0.5f; // Define marble radius globally

// Restitution constants (bounciness)
const float restitution_ground = 0.5f; // How much energy is kept on ground bounce (0=no bounce, 1=perfect bounce)
const float restitution_wall = 0.4f;   // How much energy is kept on wall bounce

// --- Extern Global Variables ---
// Marble position and velocity
extern float marbleX, marbleZ;
extern float marbleVX, marbleVZ;
extern float marbleY, marbleVY; // Add these

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
extern bool specialKeyStates[256];

// Arena Geometry Data (Heightmap)
// Deklarasi ini sekarang valid karena GRID_SIZE adalah konstanta yang diketahui
extern float arenaHeights[GRID_SIZE][GRID_SIZE];

// Checkpoint System
struct Vec3 {
    float x, y, z;
};

extern std::vector<Vec3> checkpoints;
extern int activeCheckpointIndex;

extern GLuint marbleTextureID; // For the marble texture
extern GLUquadric* sphereQuadric; // For drawing textured spheres

#endif // GLOBALS_H
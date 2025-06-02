#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glut.h>
#include <vector>

struct GLUquadric;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

const int GRID_SIZE = 150;         
const float BOUNDS = 40.0f;
const float minGroundHeight = -20.0f; 
const float defaultFallingHeight = -8.0f;
const float pathBaseHeight = 0.5f;

const float gravity = 25.0f;   
const float friction = 0.98f;  
const float deltaTime = 0.016f; 
const float inputPushForce = 30.0f;

const float mouseSensitivity = 0.4f;

const float checkpointRadius = 1.5f;

const float marbleRadius = 0.5f; 

const float restitution_ground = 0.5f; 
const float restitution_wall = 0.4f;  

extern float marbleX, marbleZ;
extern float marbleVX, marbleVZ;
extern float marbleY, marbleVY; 

extern float cameraAngleX;
extern float cameraAngleY;
extern float cameraDistance;
extern float cameraTargetYOffset;

extern bool isDragging;
extern int lastMouseX, lastMouseY;

extern bool keyStates[256];
extern bool specialKeyStates[256];

extern float arenaHeights[GRID_SIZE][GRID_SIZE];

struct Vec3 {
    float x, y, z;
};

struct CheckpointData {
    Vec3 position;
    float bonusMinutes;
};

extern std::vector<CheckpointData> checkpointData;
extern std::vector<Vec3> checkpoints;
extern int activeCheckpointIndex;
extern std::vector<bool> checkpointCollected;

extern GLuint marbleTextureID; 
extern GLUquadric* sphereQuadric; 

extern int score;

extern bool enableShadows;

void initGame(); 
#endif // GLOBALS_H
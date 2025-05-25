#ifndef ARENA_H
#define ARENA_H

// Initialization
void initArena();

// New 3D Arena Functions
void setupArenaGeometry();
void addCube3D(float x, float y, float z, float width, float height, float depth, float r = 0.8f, float g = 0.2f, float b = 0.2f);
void addTriangle3D(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float r = 0.2f, float g = 0.8f, float b = 0.2f);
void addPlatform(float x, float y, float z, float width, float depth, float height, float r = 0.6f, float g = 0.6f, float b = 0.6f);
void addTriangleRamp(float startX, float startZ, float endX, float endZ, float startHeight, float endHeight, float width, float r = 1.0f, float g = 1.0f, float b = 0.2f);

// Physics functions untuk 3D objects
float getFloorHeight(float x, float z, float currentMarbleY = 0.0f);
bool checkCollisionWithObjects(float x, float y, float z, float radius);

// Legacy compatibility functions
float getArenaHeight(float x, float z);
void getArenaHeightAndNormal(float x, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ);

// Drawing functions
void draw3DArena();
void drawGround();

#endif // ARENA_H
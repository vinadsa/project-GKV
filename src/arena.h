#ifndef ARENA_H
#define ARENA_H


// Function Declarations for arena
void setupArenaGeometry();
void drawCube(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ);
void drawRamp(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ, char axis);
void drawBush(float centerX, float centerY, float centerZ, float radius);
float getArenaHeight(float x, float z);
void getArenaHeightAndNormal(float x, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ);
void drawGround();
float getArenaHeightAt(float x, float y, float z);
void getArenaHeightAndNormalAt(float x, float y, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ);

void drawTree(float x, float y, float z, float trunkHeight, float trunkRadius, float foliageRadius);
#endif // ARENA_H
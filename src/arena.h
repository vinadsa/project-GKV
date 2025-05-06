#ifndef ARENA_H
#define ARENA_H

// Function Declarations for arena
void setupArenaGeometry();
void addFlatArea(float centerX, float centerZ, float width, float depth, float height);
void addRampArea(float centerX, float centerZ, float width, float depth, float startHeight, float endHeight, char axis);
float getArenaHeight(float x, float z);
void getArenaHeightAndNormal(float x, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ);
void drawGround();

#endif // ARENA_H
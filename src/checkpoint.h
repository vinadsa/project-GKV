#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "globals.h" // For Vec3, std::vector

void addCheckpoint(float x, float z);
void checkCheckpointCollision();
void resetMarble(); // Centralized reset logic
void setupCheckpoints(); // To add all checkpoints in one place
void drawCheckpoints();
#endif // CHECKPOINT_H
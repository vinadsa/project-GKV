#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "globals.h" 

void addCheckpoint(float x, float z, float bonusMinutes = 1.0f);
void checkCheckpointCollision();
void resetMarble(); 
void setupCheckpoints(); 
void drawCheckpoints();
void addFinish(float x, float z);
void checkFinishCollision();
void drawFinish();
#endif // CHECKPOINT_H
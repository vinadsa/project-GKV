#include "checkpoint.h"
#include "globals.h" // Sekarang menyertakan checkpointRadius, Vec3, std::vector, BOUNDS
#include "arena.h"   // For getArenaHeightAndNormal
#include "marble.h"  // For marbleX, marbleZ, etc. (accessing via globals)
#include <vector>
#include <cmath>     // For fabs, sqrt
#include <iostream>  // For std::cout

// Variabel global dari globals.h yang terutama terkait checkpoint
std::vector<Vec3> checkpoints;
int activeCheckpointIndex = -1;
// const float checkpointRadius = 1.5f; // Dipindahkan ke globals.h

// extern float totalRotationAngleX; // If statics in marble.cpp are made global for reset
// extern float totalRotationAngleZ; // Or provide a reset function in marble.h

void addCheckpoint(float x, float z) {
    checkpoints.push_back({x, 0.0f, z}); // Y is placeholder
}

void checkCheckpointCollision() {
    for (int i = activeCheckpointIndex + 1; i < checkpoints.size(); ++i) {
        Vec3 cp = checkpoints[i];
        float cpGroundH, dummyNX, dummyNY, dummyNZ;
        getArenaHeightAndNormal(cp.x, cp.z, cpGroundH, dummyNX, dummyNY, dummyNZ);
        float cpY = cpGroundH + 0.5f;

        float dx = marbleX - cp.x;
        float dz = marbleZ - cp.z;
        float distSq = dx * dx + dz * dz;

        if (distSq < checkpointRadius * checkpointRadius) {
            float marbleCurrentY = getArenaHeight(marbleX, marbleZ) + 0.5f;
            if (fabs(marbleCurrentY - cpY) < 2.0f) {
                activeCheckpointIndex = i;
                std::cout << "Checkpoint " << i + 1 << " activated!" << std::endl;
            }
        }
    }
}

void resetMarble() {
    Vec3 resetPos;
    if (activeCheckpointIndex >= 0 && activeCheckpointIndex < checkpoints.size()) {
        resetPos = checkpoints[activeCheckpointIndex];
        std::cout << "Resetting to Checkpoint " << activeCheckpointIndex + 1 << std::endl;
    } else if (!checkpoints.empty()) {
        resetPos = checkpoints[0];
         std::cout << "Resetting to Start (Checkpoint 1)" << std::endl;
    } else {
        // Fallback if no checkpoints are added - use a hardcoded start from marble.cpp's initial state idea
        resetPos = {0.0f, 0.0f, -BOUNDS + 2.0f}; // Ensure BOUNDS is accessible or use a local const
        std::cout << "Resetting to Fallback Start" << std::endl;
    }

    marbleX = resetPos.x;
    marbleZ = resetPos.z;
    float resetGroundH, dummyNX, dummyNY, dummyNZ;
    getArenaHeightAndNormal(marbleX, marbleZ, resetGroundH, dummyNX, dummyNY, dummyNZ);
    marbleY = resetGroundH + 0.5f; // Assuming marble radius 0.5f

    marbleVX = 0.0f;
    marbleVZ = 0.0f;
    marbleVY = 0.0f;

    // To reset visual rolling, either make totalRotationAngleX/Z global
    // or call a specific reset function from marble.h/cpp.
    // For now, assuming resetMarbleInitialState() in marble.cpp handles its own statics
    // and this function is for checkpoint-based resets.
    // If totalRotationAngleX/Z are file static in marble.cpp, they won't be reset here
    // unless marble.cpp provides a function to do so, or they are made extern.
    // For simplicity, if you call resetMarbleInitialState() from your game's global reset,
    // that will handle the visual part for a full game reset.
}

void setupCheckpoints() {
    // Ensure BOUNDS is accessible if used directly or pass it if it's not global from globals.h
    addCheckpoint(0.0f, -BOUNDS + 2.0f);
    addCheckpoint(0.0f, -BOUNDS + 7.0f);
    addCheckpoint(0.0f, -BOUNDS + 12.0f);
    addCheckpoint(0.0f, -BOUNDS + 16.0f);
    addCheckpoint(-BOUNDS + 4.0f, -BOUNDS + 16.0f);
    addCheckpoint(-BOUNDS + 12.0f, -BOUNDS + 12.0f);
}
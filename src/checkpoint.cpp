#include "checkpoint.h"
#include "globals.h" // Sekarang menyertakan checkpointRadius, Vec3, std::vector, BOUNDS
#include "arena.h"   // For getArenaHeightAndNormal
#include "marble.h"  // For marbleX, marbleZ, etc. (accessing via globals)
#include "timer.h"   // Added to access recordCheckpointTime()
#include <vector>
#include <cmath>     // For fabs, sqrt
#include <iostream>  // For std::cout
#include <GL/glut.h>

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
                recordCheckpointTime(); // Call function to record checkpoint time
            }
        }
    }
}

void drawCheckpoints() {
    // Aktifkan blending untuk checkpoint yang sudah diambil (transparan)
    // Untuk checkpoint yang belum diambil, kita akan menonaktifkan blend sementara
    // agar warna kuningnya solid.
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT); // Simpan state GL
    glEnable(GL_LIGHTING); // Pastikan lighting aktif untuk checkpoint

    for (size_t i = 0; i < checkpoints.size(); ++i) {
        const Vec3& cp_data = checkpoints[i]; // cp_data.y adalah Y yang disimpan saat addCheckpoint (saat ini 0.0f)

        float cpGroundH, dummyNX, dummyNY, dummyNZ;
        // Dapatkan ketinggian tanah di lokasi XZ checkpoint
        getArenaHeightAndNormal(cp_data.x, cp_data.z, cpGroundH, dummyNX, dummyNY, dummyNZ);
        
        // Radius visual checkpoint adalah setengah dari radius bola pemain
        float visualCheckpointRadius = marbleRadius * 0.5f;
        // Y efektif untuk visual checkpoint (pusat bola checkpoint)
        float cpEffectiveY = cpGroundH + visualCheckpointRadius;

        glPushMatrix();
        glTranslatef(cp_data.x, cpEffectiveY, cp_data.z);

        if ((int)i > activeCheckpointIndex) {
            // Checkpoint belum diambil: warna kuning solid
            glEnable(GL_BLEND); // Nonaktifkan blending untuk warna solid
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Atur fungsi blending
            glColor4f(1.0f, 1.0f, 0.0f, 0.5f); // Kuning solid dengan alpha 0.5
        } else {
            // Checkpoint sudah diambil: transparan
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.6f, 0.6f, 0.6f, 0.25f); // Abu-abu dengan alpha 0.25 (cukup transparan)
        }
        
        glutSolidSphere(visualCheckpointRadius, 16, 16); // Gambar bola checkpoint

        glPopMatrix();
    }
    glPopAttrib(); // Kembalikan state GL
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

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~buat checkpoint di sini~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    addCheckpoint(0.0f, -8.0f);
    addCheckpoint(0.0f, -4.0f);
    addCheckpoint(-6.0f, -4.5f);
}
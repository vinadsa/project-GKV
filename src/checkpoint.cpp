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
std::vector<bool> checkpointCollected; // Track which checkpoints have been collected


void addCheckpoint(float x, float z, float bonusMinutes) {
    CheckpointData data;
    data.position = {x, 0.0f, z}; // Y is placeholder
    data.bonusMinutes = bonusMinutes;
    
    checkpointData.push_back(data);
    checkpoints.push_back({x, 0.0f, z}); // Keep backward compatibility
    checkpointCollected.push_back(false); // Initialize as not collected
}

void checkCheckpointCollision() {
    // Check all checkpoints, not just those after activeCheckpointIndex
    for (int i = 0; i < checkpoints.size(); ++i) {
        // Skip if this checkpoint is already collected
        if (checkpointCollected[i]) {
            continue;
        }
        
        Vec3 cp = checkpoints[i];
        float cpGroundH, dummyNX, dummyNY, dummyNZ;
        getArenaHeightAndNormal(cp.x, cp.z, cpGroundH, dummyNX, dummyNY, dummyNZ);
        float cpY = cpGroundH + 0.5f;        // Calculate 3D distance between marble and checkpoint
        float dx = marbleX - cp.x;
        float dz = marbleZ - cp.z;
        float dy = marbleY - cpY;
        float dist3D = sqrt(dx * dx + dy * dy + dz * dz);
        
        // Collision radius is the sum of marble radius and checkpoint radius
        float collisionRadius = marbleRadius + (marbleRadius * 0.5f); // checkpoint radius is marbleRadius * 0.5f
        
        if (dist3D < collisionRadius) {
                // Mark this checkpoint as collected
                checkpointCollected[i] = true;
                // Update spawn point to this checkpoint (highest index collected becomes spawn)
                if (i > activeCheckpointIndex) {
                    activeCheckpointIndex = i;
                }
                  // Only give score if this is NOT the first checkpoint (spawn point)
                if (i > 0) {
                    score += 100; // Increment score for hitting a checkpoint
                    // Add bonus time to countdown
                    double bonusSeconds = checkpointData[i].bonusMinutes * 60.0;
                    addTimeToCountdown(bonusSeconds);
                    std::cout << "Checkpoint " << i + 1 << " collected! Score +100. Time +" 
                              << checkpointData[i].bonusMinutes << " min. Spawn point updated." << std::endl;
                } else {
                    std::cout << "Spawn checkpoint collected (no score). Spawn point updated." << std::endl;
                }
                  recordCheckpointTime(); // Call function to record checkpoint time
            }
        }
    }

void drawCheckpoints() {
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
        float cpEffectiveY = cpGroundH + visualCheckpointRadius;        glPushMatrix();
        glTranslatef(cp_data.x, cpEffectiveY, cp_data.z);        // Set material properties for checkpoints
        GLfloat cp_ambient[4], cp_diffuse[4], cp_specular[4];
        GLfloat cp_shininess;
        
        if (!checkpointCollected[i]) {
            // Checkpoint belum diambil: material kuning berkilau
            cp_ambient[0] = 0.3f; cp_ambient[1] = 0.3f; cp_ambient[2] = 0.0f; cp_ambient[3] = 0.8f;
            cp_diffuse[0] = 1.0f; cp_diffuse[1] = 1.0f; cp_diffuse[2] = 0.0f; cp_diffuse[3] = 0.8f;
            cp_specular[0] = 1.0f; cp_specular[1] = 1.0f; cp_specular[2] = 0.5f; cp_specular[3] = 0.8f;
            cp_shininess = 60.0f;
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 0.0f, 0.8f); // Kuning dengan alpha 0.8
        } else {
            // Checkpoint sudah diambil: material abu-abu transparan
            cp_ambient[0] = 0.2f; cp_ambient[1] = 0.2f; cp_ambient[2] = 0.2f; cp_ambient[3] = 0.3f;
            cp_diffuse[0] = 0.6f; cp_diffuse[1] = 0.6f; cp_diffuse[2] = 0.6f; cp_diffuse[3] = 0.3f;
            cp_specular[0] = 0.3f; cp_specular[1] = 0.3f; cp_specular[2] = 0.3f; cp_specular[3] = 0.3f;
            cp_shininess = 20.0f;
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.6f, 0.6f, 0.6f, 0.3f); // Abu-abu dengan alpha 0.3
        }
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cp_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cp_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cp_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cp_shininess);
        
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
        // Only set activeCheckpointIndex = 0 if no checkpoints have been collected yet
        bool anyCheckpointCollected = false;
        for (bool collected : checkpointCollected) {
            if (collected) {
                anyCheckpointCollected = true;
                break;
            }
        }
        if (!anyCheckpointCollected) {
            activeCheckpointIndex = 0; // Set to 0 since we're spawning at the first checkpoint
        }
        std::cout << "Resetting to Start (Checkpoint 1)" << std::endl;
    } else {
        // Fallback if no checkpoints are added - use a hardcoded start from marble.cpp's initial state idea
        resetPos = {0.0f, 0.0f, -BOUNDS + 2.0f}; // Ensure BOUNDS is accessible or use a local const
        activeCheckpointIndex = -1; // No checkpoints available
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

}

void setupCheckpoints() {
    // Ensure BOUNDS is accessible if used directly or pass it if it's not global from globals.h

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~buat checkpoint di sini~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // addCheckpoint(0.0f, -8.0f, 0.0f);    // First checkpoint (spawn) - 0.0 minutes bonus
    // addCheckpoint(0.0f, -4.0f, 0.5f);    // Second checkpoint - 1 minute bonus  
    // addCheckpoint(-6.0f, -4.5f, 0.5f);   // Third checkpoint - 1.5 minutes bonus
    addCheckpoint(-10.0f, -2.0f, 0.5f);  // Fourth checkpoint - 2 minutes bonus
    addCheckpoint(9.98f, -7.84f, 0.5f);
    addCheckpoint(9.95f, 22.66f, 0.5f);
}
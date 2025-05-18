#include "marble.h"
#include "globals.h"
#include "arena.h" // For getArenaHeightAndNormal
#include "checkpoint.h" // For resetMarble (which handles checkpoint logic)
#include <GL/glut.h>
#include <GL/glu.h>     // For gluSphere

// Define global variables from globals.h that are primarily managed or used by marble logic
float marbleX = 0.0f, marbleZ = 0.0f;
float marbleVX = 0.0f, marbleVZ = 0.0f;
float marbleY = 0.0f; // Define and initialize
float marbleVY = 0.0f; // Define and initialize

// For visual rolling (see section 2)
static float totalRotationAngleX = 0.0f;
static float totalRotationAngleZ = 0.0f;

void drawMarble() {
    glPushMatrix();
    // Use the global marbleX, marbleY, marbleZ which are updated by physics
    glTranslatef(marbleX, marbleY, marbleZ);

    // Visual Rolling Logic (see section 2)
    const float marbleRadius = 0.5f;
    if (marbleRadius > 1e-6f && deltaTime > 0.0f) {
        float deltaAngleX = (marbleVZ * deltaTime / marbleRadius) * (180.0f / 3.1415926535f);
        float deltaAngleZ = (-marbleVX * deltaTime / marbleRadius) * (180.0f / 3.1415926535f);
        totalRotationAngleX += deltaAngleX;
        totalRotationAngleZ += deltaAngleZ;
    }
    glRotatef(totalRotationAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(totalRotationAngleZ, 0.0f, 0.0f, 1.0f);

    if (marbleTextureID != 0 && sphereQuadric != nullptr) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, marbleTextureID);

        // Set material color to white if GL_COLOR_MATERIAL is enabled,
        // so texture colors are not tinted by the current glColor.
        glColor3f(1.0f, 1.0f, 1.0f);

        gluSphere(sphereQuadric, 0.5, 32, 32); // Draw sphere with texture

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
        glDisable(GL_TEXTURE_2D);
    } else {
        // Fallback to untextured sphere if texture loading failed or quadric not available
        glColor3f(1.0f, 0.1f, 0.1f); // Original color
        glutSolidSphere(0.5, 32, 32);
    }

    glPopMatrix();
}

// This function sets the very initial state before any checkpoints are considered.
// The main reset logic tied to checkpoints is in checkpoint.cpp
void resetMarbleInitialState() {
    marbleX = 0.0f;
    marbleZ = -BOUNDS + 2.0f;
    float startGroundH, dummyNX, dummyNY, dummyNZ;
    getArenaHeightAndNormal(marbleX, marbleZ, startGroundH, dummyNX, dummyNY, dummyNZ);
    marbleY = startGroundH + 0.5f; // Assuming marble radius 0.5f
    marbleVX = 0.0f;
    marbleVZ = 0.0f;
    marbleVY = 0.0f;
    totalRotationAngleX = 0.0f; // Reset visual rotation
    totalRotationAngleZ = 0.0f; // Reset visual rotation
}
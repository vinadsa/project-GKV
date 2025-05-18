#include "marble.h"
#include "globals.h"
#include "arena.h" // For getArenaHeightAndNormal
#include "checkpoint.h" // For resetMarble (which handles checkpoint logic)
#include <GL/glut.h>
#include <GL/glu.h>     // For gluSphere

// Define global variables from globals.h that are primarily managed or used by marble logic
float marbleX = 0.0f, marbleZ = 0.0f;
float marbleVX = 0.0f, marbleVZ = 0.0f;

void drawMarble() {
    float groundH, dummyNX, dummyNY, dummyNZ;
    getArenaHeightAndNormal(marbleX, marbleZ, groundH, dummyNX, dummyNY, dummyNZ);
    float marbleY = groundH + 0.5f; // Assuming marble radius is 0.5

    glPushMatrix();
    glTranslatef(marbleX, marbleY, marbleZ);

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
    // This initial position will be overridden by resetMarble() if checkpoints exist.
    // It serves as a fallback if no checkpoints are defined.
    marbleX = 0.0f;
    marbleZ = -BOUNDS + 2.0f; // Default starting Z
    marbleVX = 0.0f;
    marbleVZ = 0.0f;
}
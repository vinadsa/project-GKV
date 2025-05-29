#include "marble.h"
#include "globals.h"
#include "arena.h" // For getArenaHeightAndNormal
#include "checkpoint.h" // For resetMarble (which handles checkpoint logic)
#include <GL/glut.h>
#include <GL/glu.h>     // For gluSphere

int score = 0; // Initialize score variable
// This file contains the marble logic, including its position, velocity, and rendering.

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
        float deltaAngleZ = (marbleVX * deltaTime / marbleRadius) * (180.0f / 3.1415926535f); // Changed -marbleVX to marbleVX
        totalRotationAngleX += deltaAngleX;
        totalRotationAngleZ += deltaAngleZ;
    }
    glRotatef(totalRotationAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(totalRotationAngleZ, 0.0f, 0.0f, 1.0f);    if (marbleTextureID != 0 && sphereQuadric != nullptr) {
        // For textured marble: disable GL_COLOR_MATERIAL temporarily to use explicit materials
        glDisable(GL_COLOR_MATERIAL);
        
        // Set material properties that preserve texture visibility
        GLfloat marble_ambient[] = {0.6f, 0.6f, 0.6f, 1.0f};    // Higher ambient to preserve texture
        GLfloat marble_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};    // White diffuse for pure texture colors
        GLfloat marble_specular[] = {0.3f, 0.3f, 0.3f, 1.0f};   // Reduced specular to prevent washout
        GLfloat marble_shininess = 30.0f;                        // Lower shininess
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, marble_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, marble_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, marble_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, marble_shininess);
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, marbleTextureID);

        gluSphere(sphereQuadric, 0.5, 32, 32); // Draw sphere with texture

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
        glDisable(GL_TEXTURE_2D);
        
        // Re-enable GL_COLOR_MATERIAL for other objects
        glEnable(GL_COLOR_MATERIAL);
    } else {
        // For untextured marble: use GL_COLOR_MATERIAL with glColor
        glColor3f(0.9f, 0.7f, 0.5f); // Marble-like color (this will set ambient and diffuse due to GL_COLOR_MATERIAL)
        
        // Set only specular and shininess explicitly
        GLfloat marble_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};   // High specular for shininess
        GLfloat marble_shininess = 80.0f;                        // High shininess value
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, marble_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, marble_shininess);
        
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
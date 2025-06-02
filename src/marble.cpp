#include "marble.h"
#include "globals.h"
#include "arena.h" // For getArenaHeightAndNormal
#include "checkpoint.h" // For resetMarble (which handles checkpoint logic)
#include <GL/glut.h>
#include <GL/glu.h>     // For gluSphere

int score = 0; // Initialize score variable

// Define checkpoint data storage
std::vector<CheckpointData> checkpointData; // Store checkpoint position and bonus time

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
    glRotatef(totalRotationAngleZ, 0.0f, 0.0f, 1.0f);    
    
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, marbleTextureID); // Bind the loaded marble texture

    // Disable color material when using textures to ensure material properties are used correctly
    glDisable(GL_COLOR_MATERIAL);

    // Set material properties for the textured marble
    GLfloat marble_ambient[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Ambient should be high if texture is pre-lit or to make it generally bright
    GLfloat marble_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Diffuse to 1 means texture color is fully used
    GLfloat marble_specular[] = {0.7f, 0.7f, 0.7f, 1.0f}; // Moderate specular highlight
    GLfloat marble_shininess = 50.0f;                     // Moderate shininess

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, marble_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, marble_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, marble_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, marble_shininess);
    
    if (sphereQuadric != nullptr) { // Check if sphereQuadric is initialized
        gluSphere(sphereQuadric, 0.5, 32, 32); // Draw sphere using quadric (which should have texture coords enabled)
    } else {
        glColor3f(0.9f, 0.7f, 0.5f); // Fallback color
        glutSolidSphere(0.5, 32, 32); 
    }

    // Disable texturing and re-enable color material for other objects if needed
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL); // Re-enable for other objects that might use it

    glPopMatrix();
}


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
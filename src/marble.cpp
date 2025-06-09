#include "marble.h"
#include "globals.h"
#include "arena.h"
#include "checkpoint.h"
#include <GL/glut.h>
#include <GL/glu.h>

int score = 0;

std::vector<CheckpointData> checkpointData; 

float marbleX = 0.0f, marbleZ = 0.0f;
float marbleVX = 0.0f, marbleVZ = 0.0f;
float marbleY = 0.0f; 
float marbleVY = 0.0f; 

static float totalRotationAngleX = 0.0f;
static float totalRotationAngleZ = 0.0f;

void drawMarble() {
    glPushMatrix();
    glTranslatef(marbleX, marbleY, marbleZ);

    const float marbleRadius = 0.5f;
    
    if (marbleRadius > 1e-6f && deltaTime > 0.0f) {
        float deltaAngleX = (marbleVZ * deltaTime / marbleRadius) * (180.0f / 3.1415926535f);
        float deltaAngleZ = (marbleVX * deltaTime / marbleRadius) * (180.0f / 3.1415926535f); // Changed -marbleVX to marbleVX
        totalRotationAngleX += deltaAngleX;
        totalRotationAngleZ += deltaAngleZ;
    }
    glRotatef(totalRotationAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(totalRotationAngleZ, 0.0f, 0.0f, 1.0f);    
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, marbleTextureID);

    glDisable(GL_COLOR_MATERIAL);

    GLfloat marble_ambient[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat marble_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    GLfloat marble_specular[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat marble_shininess = 50.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, marble_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, marble_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, marble_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, marble_shininess);
    
    if (sphereQuadric != nullptr) {
        gluSphere(sphereQuadric, 0.5, 32, 32);
    } else {
        glColor3f(0.9f, 0.7f, 0.5f);
        glutSolidSphere(0.5, 32, 32); 
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL); 

    glPopMatrix();
}


void resetMarbleInitialState() {
    marbleX = 0.0f;
    marbleZ = -BOUNDS + 2.0f;
    float startGroundH, dummyNX, dummyNY, dummyNZ;
    getArenaHeightAndNormal(marbleX, marbleZ, startGroundH, dummyNX, dummyNY, dummyNZ);
    marbleY = startGroundH + 0.5f;
    marbleVX = 0.0f;
    marbleVZ = 0.0f;
    marbleVY = 0.0f;
    totalRotationAngleX = 0.0f;
    totalRotationAngleZ = 0.0f;
}
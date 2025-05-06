#include "graphics.h"
#include "globals.h"
#include "arena.h"    // For drawGround, getArenaHeightAndNormal
#include "marble.h"   // For drawMarble, marbleX, marbleZ (accessing via globals)
#include "camera.h"   // For camera variables (accessing via globals)
#include "physics.h"  // For updatePhysics
#include "utils.h"    // For degToRad
#include <GL/glut.h>
#include <cmath>     // For cos, sin

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float targetX = marbleX;
    float targetGroundY, dummyNX, dummyNY, dummyNZ;
    getArenaHeightAndNormal(marbleX, marbleZ, targetGroundY, dummyNX, dummyNY, dummyNZ);
    float targetY = targetGroundY + cameraTargetYOffset;
    float targetZ = marbleZ;

    float camAngleXRad = degToRad(cameraAngleX);
    float camAngleYRad = degToRad(cameraAngleY);

    float camX = targetX + cameraDistance * cos(camAngleYRad) * sin(camAngleXRad);
    float camY = targetY + cameraDistance * sin(camAngleYRad);
    float camZ = targetZ + cameraDistance * cos(camAngleYRad) * cos(camAngleXRad);

    gluLookAt(camX, camY, camZ,
              targetX, targetY, targetZ,
              0.0, 1.0, 0.0);

    drawGround();
    drawMarble();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int value) {
    updatePhysics();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void initGraphics() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat light_pos[] = {0.0f, 30.0f, 30.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    GLfloat diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Sky blue
}
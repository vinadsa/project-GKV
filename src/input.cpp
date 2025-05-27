#include "input.h"
#include "globals.h"  // Sekarang menyertakan mouseSensitivity
#include "utils.h"    // For clamp
#include "checkpoint.h" // For resetMarble
#include <GL/glut.h>
#include <cstdlib> // For exit()

// Variabel global dari globals.h yang terutama terkait input
bool isDragging = false;
int lastMouseX = 0, lastMouseY = 0;
bool keyStates[256]; // Definisi


void initKeyStates() {
    for (int i = 0; i < 256; ++i) {
        keyStates[i] = false;
    }
}

void specialKeysDown(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    keyStates[GLUT_KEY_UP] = true; break;
        case GLUT_KEY_DOWN:  keyStates[GLUT_KEY_DOWN] = true; break;
        case GLUT_KEY_LEFT:  keyStates[GLUT_KEY_LEFT] = true; break;
        case GLUT_KEY_RIGHT: keyStates[GLUT_KEY_RIGHT] = true; break;
    }
}

void specialKeysUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    keyStates[GLUT_KEY_UP] = false; break;
        case GLUT_KEY_DOWN:  keyStates[GLUT_KEY_DOWN] = false; break;
        case GLUT_KEY_LEFT:  keyStates[GLUT_KEY_LEFT] = false; break;
        case GLUT_KEY_RIGHT: keyStates[GLUT_KEY_RIGHT] = false; break;
    }
}

void normalKeysDown(unsigned char key, int x, int y) {
    if (key < 256) {
        keyStates[key] = true;
    }
    if (key == 27) { // ESC
        exit(0);
    }
    if (key == 'r' || key == 'R') {
        resetMarble(); // Calls the function from checkpoint.cpp
    }
    // --- Print marble position for placement helper ---
    if (key == 'o' || key == 'O') {
        extern float marbleX, marbleY, marbleZ;
        extern void PrintMarblePositionForPlacement(float x, float y, float z);
        PrintMarblePositionForPlacement(marbleX, marbleY, marbleZ);
    }
}

void normalKeysUp(unsigned char key, int x, int y) {
    if (key < 256) {
        keyStates[key] = false;
    }
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            isDragging = false;
        }
    } else if (button == 3) { // Scroll up
        cameraDistance -= 0.5f;
        cameraDistance = clamp(cameraDistance, 2.0f, 30.0f);
        glutPostRedisplay();
    } else if (button == 4) { // Scroll down
        cameraDistance += 0.5f;
        cameraDistance = clamp(cameraDistance, 2.0f, 30.0f);
        glutPostRedisplay();
    }
}

void mouseMove(int x, int y) {
    if (isDragging) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        cameraAngleX += deltaX * mouseSensitivity;
        cameraAngleY += deltaY * mouseSensitivity;
        cameraAngleY = clamp(cameraAngleY, 5.0f, 85.0f);

        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}
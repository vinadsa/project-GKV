#include <GL/glut.h>
#include <cmath>         
#include "globals.h"
#include "utils.h"
#include "arena.h"
#include "marble.h"
#include "physics.h"
#include "camera.h"
#include "input.h"
#include "checkpoint.h"
#include "graphics.h" 
#include "timer.h"

void initGame() {
    initKeyStates();
    setupArenaGeometry();
    setupCheckpoints();
    resetMarbleInitialState();
    score = 0; 
    activeCheckpointIndex = -1;
    resetMarble();
    initGraphics();
    resetTimer();
    initCountdownTimer(60.0);
    startTimer();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Marble Arena Game - Refactored");

    initGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutSpecialFunc(specialKeysDown);
    glutSpecialUpFunc(specialKeysUp);
    glutKeyboardFunc(normalKeysDown);
    glutKeyboardUpFunc(normalKeysUp);
    glutIgnoreKeyRepeat(1);

    glutMainLoop();
    return 0;
}
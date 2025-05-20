#include <GL/glut.h>
#include <cmath>         // For trigonometric functions like sin() and cos()
#include "globals.h"      // For global variables and M_PI (though M_PI might be better in utils.h if not widely needed)
#include "utils.h"
#include "arena.h"
#include "marble.h"
#include "physics.h"
#include "camera.h"       // Not strictly needed if only globals are used from it
#include "input.h"
#include "checkpoint.h"
#include "graphics.h"     // For display, reshape, timer, initGraphics

void initGame() {
    initKeyStates();         // Initialize keyboard states
    setupArenaGeometry();    // Define the arena layout
    setupCheckpoints();      // Define checkpoints after arena is built
    resetMarbleInitialState(); // Set a very basic marble position
    activeCheckpointIndex = -1; // Ensure we start fresh with checkpoints
    resetMarble();           // Reset marble to the first checkpoint (or defined start)
    initGraphics();          // Initialize OpenGL specific settings (lighting, depth test, etc.)
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Marble Arena Game - Refactored");

    initGame(); // Call our game initialization function

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
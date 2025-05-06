#include "physics.h"
#include "globals.h"  // Sekarang menyertakan gravity, friction, deltaTime, inputPushForce
#include "utils.h"    // For clamp, degToRad
#include "arena.h"    // For getArenaHeightAndNormal
#include "marble.h"   // For marbleX, marbleZ etc. (accessing via globals)
#include "checkpoint.h" // For resetMarble, checkCheckpointCollision
#include <cmath>      // For sqrt, fabs, cos, sin
#include <GL/glut.h>  // For keyStates and GLUT_KEY_* constants

void updatePhysics() {
    float groundHeight, normalX, normalY, normalZ;
    getArenaHeightAndNormal(marbleX, marbleZ, groundHeight, normalX, normalY, normalZ);

    if (groundHeight < minGroundHeight) {
        resetMarble(); // This now calls the function from checkpoint.cpp
        return;
    }

    checkCheckpointCollision();

    float gravityVecY = -gravity;
    float dot_gravity_normal = (gravityVecY * normalY);

    float gravityForceX = -dot_gravity_normal * normalX;
    float gravityForceZ = -dot_gravity_normal * normalZ;

    float accX = gravityForceX;
    float accZ = gravityForceZ;

    float camAngleXRad = degToRad(cameraAngleX);
    float cosCam = cos(camAngleXRad);
    float sinCam = sin(camAngleXRad);

    float inputDirX = 0.0f;
    float inputDirZ = 0.0f;
    if (keyStates[GLUT_KEY_UP])    { inputDirX -= sinCam; inputDirZ -= cosCam; }
    if (keyStates[GLUT_KEY_DOWN])  { inputDirX += sinCam; inputDirZ += cosCam; }
    if (keyStates[GLUT_KEY_LEFT])  { inputDirX -= cosCam; inputDirZ += sinCam; }
    if (keyStates[GLUT_KEY_RIGHT]) { inputDirX += cosCam; inputDirZ -= sinCam; }

    float inputMagnitude = sqrt(inputDirX * inputDirX + inputDirZ * inputDirZ);
    if (inputMagnitude > 1e-6) {
        inputDirX /= inputMagnitude;
        inputDirZ /= inputMagnitude;

        float slopeFactor = clamp(normalY, 0.0f, 1.0f);
        float effectivePushForce = inputPushForce * slopeFactor;

        accX += inputDirX * effectivePushForce;
        accZ += inputDirZ * effectivePushForce;
    }

    marbleVX += accX * deltaTime;
    marbleVZ += accZ * deltaTime;

    marbleVX *= friction;
    marbleVZ *= friction;

    marbleX += marbleVX * deltaTime;
    marbleZ += marbleVZ * deltaTime;

    if (fabs(marbleVX) < 0.005f && fabs(accX) < 0.005f) marbleVX = 0.0f;
    if (fabs(marbleVZ) < 0.005f && fabs(accZ) < 0.005f) marbleVZ = 0.0f;
}
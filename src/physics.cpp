#include "physics.h"
#include "globals.h"  // Sekarang menyertakan gravity, friction, deltaTime, inputPushForce
#include "utils.h"    // For clamp, degToRad
#include "arena.h"    // For getArenaHeightAndNormal
#include "marble.h"   // For marbleX, marbleZ etc. (accessing via globals)
#include "checkpoint.h" // For resetMarble, checkCheckpointCollision
#include <cmath>      // For sqrt, fabs, cos, sin
#include <GL/glut.h>  // For keyStates and GLUT_KEY_* constants

void updatePhysics() {
    float initialGroundHeight, normalX, normalY, normalZ;
    // Get normal at current XZ for slope calculations
    getArenaHeightAndNormal(marbleX, marbleZ, initialGroundHeight, normalX, normalY, normalZ);

    // Check for falling out of bounds (use marbleY)
    // minGroundHeight should be a very low value, e.g., -50.0f
    if (marbleY < minGroundHeight) {
        resetMarble(); // This now calls the function from checkpoint.cpp
        return;
    }

    checkCheckpointCollision();

    // --- Vertical Physics ---
    marbleVY -= gravity * deltaTime; // Apply gravity (gravity should be a positive value like 9.8)

    // --- Horizontal Physics (Forces based on slope from initial position) ---
    float gravityVecY = -gravity;
    float dot_gravity_normal = (gravityVecY * normalY);
    float gravityForceX = -dot_gravity_normal * normalX;
    float gravityForceZ = -dot_gravity_normal * normalZ;

    float accX = gravityForceX;
    float accZ = gravityForceZ;

    // --- Player Input (already exists) ---
    // ... (input code using normalY for slopeFactor) ...
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

        float slopeFactor = clamp(normalY, 0.0f, 1.0f); // Use normalY from start of frame
        float effectivePushForce = inputPushForce * slopeFactor;

        accX += inputDirX * effectivePushForce;
        accZ += inputDirZ * effectivePushForce;
    }
    // ...existing code...

    // --- Update Velocities ---
    marbleVX += accX * deltaTime;
    marbleVZ += accZ * deltaTime;

    // Apply friction (damping)
    marbleVX *= friction;
    marbleVZ *= friction;
    // Consider a small air_friction for marbleVY if desired: marbleVY *= air_friction_factor;

    // --- Horizontal Collision Detection & Response ---
    const float wall_slope_normal_Y_threshold = 0.5f; // Normals with Y component less than this are "walls"
    const float collision_check_offset = 0.01f; // Small offset to prevent floating point issues

    // Check X-direction movement
    if (marbleVX != 0.0f) {
        float potential_next_marbleX = marbleX + marbleVX * deltaTime;
        float ground_h_at_next_X, norm_x_at_next_X, norm_y_at_next_X, norm_z_at_next_X;
        getArenaHeightAndNormal(potential_next_marbleX, marbleZ, ground_h_at_next_X, norm_x_at_next_X, norm_y_at_next_X, norm_z_at_next_X);

        bool is_wall_slope_X = (norm_y_at_next_X < wall_slope_normal_Y_threshold);
        // Check if the marble's current bottom would be lower than the potential new ground height
        bool would_collide_height_X = (marbleY - marbleRadius < ground_h_at_next_X - collision_check_offset);

        if (is_wall_slope_X && would_collide_height_X) {
            marbleVX = 0.0f; // Stop X-motion
        }
    }

    // Check Z-direction movement
    if (marbleVZ != 0.0f) {
        float potential_next_marbleZ = marbleZ + marbleVZ * deltaTime;
        float ground_h_at_next_Z, norm_x_at_next_Z, norm_y_at_next_Z, norm_z_at_next_Z;
        getArenaHeightAndNormal(marbleX, potential_next_marbleZ, ground_h_at_next_Z, norm_x_at_next_Z, norm_y_at_next_Z, norm_z_at_next_Z);

        bool is_wall_slope_Z = (norm_y_at_next_Z < wall_slope_normal_Y_threshold);
        bool would_collide_height_Z = (marbleY - marbleRadius < ground_h_at_next_Z - collision_check_offset);

        if (is_wall_slope_Z && would_collide_height_Z) {
            marbleVZ = 0.0f; // Stop Z-motion
        }
    }

    // --- Update Positions --- (marbleVX/VZ might be zeroed now)
    marbleX += marbleVX * deltaTime;
    marbleZ += marbleVZ * deltaTime;
    marbleY += marbleVY * deltaTime; // Update marbleY using its velocity

    // --- Collision Response (Ground) ---
    const float marbleRadius = 0.5f;
    float currentGroundHeight, newNormalX, newNormalY, newNormalZ;
    getArenaHeightAndNormal(marbleX, marbleZ, currentGroundHeight, newNormalX, newNormalY, newNormalZ);

    if (marbleY < currentGroundHeight + marbleRadius) {
        marbleY = currentGroundHeight + marbleRadius;
        // Simple stop on collision. For bounce, reflect marbleVY based on newNormalY.
        // float dot_vy_normal = marbleVY * newNormalY; // Simplified for vertical only
        // if (dot_vy_normal < 0) marbleVY = -dot_vy_normal * restitution_factor; else marbleVY = 0;
        marbleVY = 0.0f;
        
        // Optional: If on ground, can apply stronger friction or adjust horizontal forces
        // based on newNormalX, newNormalY, newNormalZ.
    }

    // Stop if velocity and acceleration are very small
    if (fabs(marbleVX) < 0.005f && fabs(accX) < 0.005f) marbleVX = 0.0f;
    if (fabs(marbleVZ) < 0.005f && fabs(accZ) < 0.005f) marbleVZ = 0.0f;
}
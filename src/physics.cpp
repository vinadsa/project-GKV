#include "physics.h"
#include "globals.h"  // Sekarang menyertakan gravity, friction, deltaTime, inputPushForce, restitution_ground, restitution_wall
#include "utils.h"    // For clamp, degToRad
#include "arena.h"    // For getArenaHeightAndNormal
#include "marble.h"   // For marbleX, marbleZ etc. (accessing via globals)
#include "checkpoint.h" // For resetMarble, checkCheckpointCollision
#include <cmath>      // For sqrt, fabs, cos, sin
#include <GL/glut.h>  // For keyStates and GLUT_KEY_* constants
#include <cstdio>

void updatePhysics() {
    float initialGroundHeight, normalX, normalY, normalZ;
    // Get normal at current XZ for slope calculations
    getArenaHeightAndNormalAt(marbleX, marbleY, marbleZ, initialGroundHeight, normalX, normalY, normalZ);

    // Check for falling out of bounds (use marbleY)
    if (marbleY < minGroundHeight) {
        resetMarble();
        return;
    }

    checkCheckpointCollision();

    // --- Vertical Physics ---
    marbleVY -= gravity * deltaTime;

    // --- Horizontal Physics (Forces based on slope from initial position) ---
    float gravityVecY = -gravity;
    float dot_gravity_normal = (gravityVecY * normalY);
    float gravityForceX = -dot_gravity_normal * normalX;
    float gravityForceZ = -dot_gravity_normal * normalZ;

    float accX = gravityForceX;
    float accZ = gravityForceZ;

    // --- Player Input ---
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

    // --- Update Velocities ---
    marbleVX += accX * deltaTime;
    marbleVZ += accZ * deltaTime;

    // Apply friction (damping)
    marbleVX *= friction;
    marbleVZ *= friction;
    // Consider a small air_friction for marbleVY if desired: marbleVY *= air_friction_factor;

    // --- Horizontal Collision Detection & Response ---
    const float wall_slope_normal_Y_threshold = 0.5f; // 0.5: all corners/edges (Y=0) are true walls
    const float collision_check_offset = 0.002f; // Lebih kecil, agar marble lebih dekat ke dinding
    const int CCD_SEGMENTS = 8; // Lebih rapat, agar marble tidak tembus dinding saat kecepatan tinggi
    const float R_eff = marbleRadius - 0.001f; // Effective radius for probing, slightly smaller to avoid sampling issues

    // --- Check X-direction movement ---
    float vx_before_x_collision = marbleVX;
    float vy_at_x_collision_check = marbleVY;
    float vz_at_x_collision_check = marbleVZ;
    bool collision_handled_X = false;

    if (vx_before_x_collision != 0.0f) {
        float signVX = (vx_before_x_collision > 0.0f ? 1.0f : -1.0f);

        // Start and end X-coordinates of the marble\'s leading surface during this deltaTime step
        float leading_edge_initial_X = marbleX + signVX * R_eff;
        float final_center_X_candidate = marbleX + vx_before_x_collision * deltaTime;
        float leading_edge_final_X = final_center_X_candidate + signVX * R_eff;

        for (int i = 0; i <= CCD_SEGMENTS; ++i) {
            float fraction = (CCD_SEGMENTS == 0) ? 1.0f : (float)i / CCD_SEGMENTS;
            // terrain_probe_X is the actual X coordinate on the terrain we are checking
            float terrain_probe_X = leading_edge_initial_X + fraction * (leading_edge_final_X - leading_edge_initial_X);

            float h_terrain, nx_terrain, ny_terrain, nz_terrain;
            getArenaHeightAndNormalAt(terrain_probe_X, marbleY, marbleZ, h_terrain, nx_terrain, ny_terrain, nz_terrain);

            bool is_wall_at_probe = (ny_terrain < wall_slope_normal_Y_threshold);
            // Check if the marble\'s bottom would go below the terrain height at the probe point
            bool would_penetrate_wall = (marbleY - R_eff < h_terrain - collision_check_offset); 
                                       // Using R_eff for consistency with probe, check if marble body hits

            if (is_wall_at_probe && would_penetrate_wall) {
                // Collision detected. The wall surface is effectively at terrain_probe_X.
                // 1. Positional Correction:
                //    Move marble\'s center so its effective edge is at terrain_probe_X.
                marbleX = terrain_probe_X - signVX * R_eff;

                // 2. Velocity Reflection:
                //    Use original velocities for reflection against the normal at the probe point.
                float reflection_ny_component = ny_terrain;
                if (is_wall_at_probe) { // If classified as a wall for collision
                    reflection_ny_component = 0.0f; // Ensure no vertical impulse from this wall
                }
                
                float v_dot_n_wall = vx_before_x_collision * nx_terrain +
                                     vy_at_x_collision_check * reflection_ny_component +
                                     vz_at_x_collision_check * nz_terrain;

                if (v_dot_n_wall < 0) { // Check if moving into the wall
                    marbleVX -= (1 + restitution_wall) * v_dot_n_wall * nx_terrain;
                    marbleVY -= (1 + restitution_wall) * v_dot_n_wall * reflection_ny_component;
                    marbleVZ -= (1 + restitution_wall) * v_dot_n_wall * nz_terrain;
                }
                collision_handled_X = true;
                break; // Collision handled for this axis, exit CCD loop
            }
        }
    }

    // --- Check Z-direction movement ---
    // (Similar CCD logic would be applied here for Z-axis)
    // Store velocities at the start of this axis check
    float vx_at_z_collision_check = marbleVX; // marbleVX might have been changed by X-collision
    float vy_at_z_collision_check = marbleVY; // marbleVY might have been changed by X-collision
    float vz_before_z_collision = marbleVZ;
    bool collision_handled_Z = false;

    if (vz_before_z_collision != 0.0f) {
        float signVZ = (vz_before_z_collision > 0.0f ? 1.0f : -1.0f);

        float leading_edge_initial_Z = marbleZ + signVZ * R_eff;
        float final_center_Z_candidate = marbleZ + vz_before_z_collision * deltaTime;
        float leading_edge_final_Z = final_center_Z_candidate + signVZ * R_eff;

        for (int i = 0; i <= CCD_SEGMENTS; ++i) {
            float fraction = (CCD_SEGMENTS == 0) ? 1.0f : (float)i / CCD_SEGMENTS;
            float terrain_probe_Z = leading_edge_initial_Z + fraction * (leading_edge_final_Z - leading_edge_initial_Z);

            float h_terrain, nx_terrain, ny_terrain, nz_terrain;
            // Pass marbleX that might have been corrected by X-collision
            getArenaHeightAndNormalAt(marbleX, marbleY, terrain_probe_Z, h_terrain, nx_terrain, ny_terrain, nz_terrain);

            bool is_wall_at_probe = (ny_terrain < wall_slope_normal_Y_threshold);
            bool would_penetrate_wall = (marbleY - R_eff < h_terrain - collision_check_offset);

            if (is_wall_at_probe && would_penetrate_wall) {
                marbleZ = terrain_probe_Z - signVZ * R_eff;

                float reflection_ny_component = ny_terrain;
                if (is_wall_at_probe) { // If classified as a wall for collision
                    reflection_ny_component = 0.0f; // Ensure no vertical impulse from this wall
                }

                float v_dot_n_wall = vx_at_z_collision_check * nx_terrain + // Use vx_at_z_collision_check
                                     vy_at_z_collision_check * reflection_ny_component + // Use vy_at_z_collision_check
                                     vz_before_z_collision * nz_terrain;
                if (v_dot_n_wall < 0) {
                    marbleVX -= (1 + restitution_wall) * v_dot_n_wall * nx_terrain;
                    marbleVY -= (1 + restitution_wall) * v_dot_n_wall * reflection_ny_component;
                    marbleVZ -= (1 + restitution_wall) * v_dot_n_wall * nz_terrain;
                }
                collision_handled_Z = true;
                break;
            }
        }
    }

    // --- Update Positions ---
    // If CCD handled collision, marbleX/Z and marbleVX/VY/VZ are already updated.
    // This final step applies the (potentially reflected) velocity from the (possibly corrected) position.
    float marbleY_before = marbleY; // Simpan posisi sebelum update
    marbleX += marbleVX * deltaTime;
    marbleZ += marbleVZ * deltaTime;
    marbleY += marbleVY * deltaTime;

    // --- Collision Response (Ground/Final) ---
    float currentGroundHeight, newNormalX, newNormalY, newNormalZ;
    getArenaHeightAndNormalAt(marbleX, marbleY, marbleZ, currentGroundHeight, newNormalX, newNormalY, newNormalZ);

    // Use 0.5 as the threshold for wall/corner detection for clarity
    const float final_collision_wall_threshold_Y = 0.5f;

    if (marbleY < currentGroundHeight + marbleRadius) { // Potential penetration or landing
        bool is_final_contact_a_wall = (newNormalY < final_collision_wall_threshold_Y);

        if (!is_final_contact_a_wall) {
            // It's a floor or a walkable slope. Perform standard ground collision response.
            marbleY = currentGroundHeight + marbleRadius; // Correct position to be on top of the surface

            float v_dot_n_ground = marbleVX * newNormalX + marbleVY * newNormalY + marbleVZ * newNormalZ;
            if (v_dot_n_ground < 0) { // If moving into the ground
                // Apply reflection using restitution_ground
                marbleVX -= (1 + restitution_ground) * v_dot_n_ground * newNormalX;
                marbleVY -= (1 + restitution_ground) * v_dot_n_ground * newNormalY;
                marbleVZ -= (1 + restitution_ground) * v_dot_n_ground * newNormalZ;
            }
        } else {
            // It's a wall or corner: DO NOT increase marbleY, and do NOT set marbleVY upwards!
            float oldY = marbleY;
            marbleY = fminf(marbleY, marbleY_before); // Never snap up
            if (marbleY > marbleY_before + 1e-4f) {
                printf("[DEBUG] Wall/corner collision tried to increase marbleY! oldY=%.4f, before=%.4f\n", marbleY, marbleY_before);
            }
            // Only apply velocity reflection (horizontal only)
            float reflection_nx_final = newNormalX;
            float reflection_ny_final = 0.0f; // Ensure no vertical impulse from wall/corner
            float reflection_nz_final = newNormalZ;
            float v_dot_n_wall = marbleVX * reflection_nx_final + 
                                 marbleVY * reflection_ny_final + 
                                 marbleVZ * reflection_nz_final;
            if (v_dot_n_wall < 0) {
                float oldVY = marbleVY;
                marbleVX -= (1 + restitution_wall) * v_dot_n_wall * reflection_nx_final;
                // Do NOT set marbleVY upwards for wall/corner
                // marbleVY -= (1 + restitution_wall) * v_dot_n_wall * reflection_ny_final; // always zero
                marbleVZ -= (1 + restitution_wall) * v_dot_n_wall * reflection_nz_final;
                if (marbleVY > oldVY + 1e-4f) {
                    printf("[DEBUG] Wall/corner collision tried to increase marbleVY! oldVY=%.4f, newVY=%.4f\n", oldVY, marbleVY);
                }
            }
        }
    }

    // Stop if velocity and acceleration are very small
    if (fabs(marbleVX) < 0.005f && fabs(accX) < 0.005f) marbleVX = 0.0f; // accX here is from start of frame
    if (fabs(marbleVZ) < 0.005f && fabs(accZ) < 0.005f) marbleVZ = 0.0f; // accZ here is from start of frame
}
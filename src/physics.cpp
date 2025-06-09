#include "physics.h"
#include "globals.h" 
#include "utils.h"   
#include "arena.h"   
#include "marble.h"  
#include "checkpoint.h"
#include <cmath>    
#include <GL/glut.h> 
#include <cstdio>

void updatePhysics() {
    float initialGroundHeight, normalX, normalY, normalZ;
    getArenaHeightAndNormalAt(marbleX, marbleY, marbleZ, initialGroundHeight, normalX, normalY, normalZ);

    if (marbleY < minGroundHeight) {
        resetMarble();
        return;
    }

    checkCheckpointCollision();
    checkFinishCollision(); 

    marbleVY -= gravity * deltaTime;

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

    const float wall_slope_normal_Y_threshold = 0.5f;
    const float collision_check_offset = 0.002f;
    const int CCD_SEGMENTS = 8; 
    const float R_eff = marbleRadius - 0.001f; 

    float vx_before_x_collision = marbleVX;
    float vy_at_x_collision_check = marbleVY;
    float vz_at_x_collision_check = marbleVZ;
    bool collision_handled_X = false;

    if (vx_before_x_collision != 0.0f) {
        float signVX = (vx_before_x_collision > 0.0f ? 1.0f : -1.0f);

        float leading_edge_initial_X = marbleX + signVX * R_eff;
        float final_center_X_candidate = marbleX + vx_before_x_collision * deltaTime;
        float leading_edge_final_X = final_center_X_candidate + signVX * R_eff;

        for (int i = 0; i <= CCD_SEGMENTS; ++i) {
            float fraction = (CCD_SEGMENTS == 0) ? 1.0f : (float)i / CCD_SEGMENTS;
            float terrain_probe_X = leading_edge_initial_X + fraction * (leading_edge_final_X - leading_edge_initial_X);

            float h_terrain, nx_terrain, ny_terrain, nz_terrain;
            getArenaHeightAndNormalAt(terrain_probe_X, marbleY, marbleZ, h_terrain, nx_terrain, ny_terrain, nz_terrain);

            bool is_wall_at_probe = (ny_terrain < wall_slope_normal_Y_threshold);
            bool would_penetrate_wall = (marbleY - R_eff < h_terrain - collision_check_offset); 

            if (is_wall_at_probe && would_penetrate_wall) {
                marbleX = terrain_probe_X - signVX * R_eff;

                float reflection_ny_component = ny_terrain;
                if (is_wall_at_probe) { 
                    reflection_ny_component = 0.0f; 
                }
                
                float v_dot_n_wall = vx_before_x_collision * nx_terrain +
                                     vy_at_x_collision_check * reflection_ny_component +
                                     vz_at_x_collision_check * nz_terrain;

                if (v_dot_n_wall < 0) { 
                    marbleVX -= (1 + restitution_wall) * v_dot_n_wall * nx_terrain;
                    marbleVY -= (1 + restitution_wall) * v_dot_n_wall * reflection_ny_component;
                    marbleVZ -= (1 + restitution_wall) * v_dot_n_wall * nz_terrain;
                }
                collision_handled_X = true;
                break; 
            }
        }
    }

    float vx_at_z_collision_check = marbleVX;
    float vy_at_z_collision_check = marbleVY;
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
            getArenaHeightAndNormalAt(marbleX, marbleY, terrain_probe_Z, h_terrain, nx_terrain, ny_terrain, nz_terrain);

            bool is_wall_at_probe = (ny_terrain < wall_slope_normal_Y_threshold);
            bool would_penetrate_wall = (marbleY - R_eff < h_terrain - collision_check_offset);

            if (is_wall_at_probe && would_penetrate_wall) {
                marbleZ = terrain_probe_Z - signVZ * R_eff;

                float reflection_ny_component = ny_terrain;
                if (is_wall_at_probe) {
                    reflection_ny_component = 0.0f;
                }

                float v_dot_n_wall = vx_at_z_collision_check * nx_terrain +
                                     vy_at_z_collision_check * reflection_ny_component +
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

    float marbleY_before = marbleY;
    marbleX += marbleVX * deltaTime;
    marbleZ += marbleVZ * deltaTime;
    marbleY += marbleVY * deltaTime;

    float currentGroundHeight, newNormalX, newNormalY, newNormalZ;
    getArenaHeightAndNormalAt(marbleX, marbleY, marbleZ, currentGroundHeight, newNormalX, newNormalY, newNormalZ);

    const float final_collision_wall_threshold_Y = 0.5f;

    if (marbleY < currentGroundHeight + marbleRadius) {
        bool is_final_contact_a_wall = (newNormalY < final_collision_wall_threshold_Y);

        if (!is_final_contact_a_wall) {
            marbleY = currentGroundHeight + marbleRadius;

            float v_dot_n_ground = marbleVX * newNormalX + marbleVY * newNormalY + marbleVZ * newNormalZ;
            if (v_dot_n_ground < 0) {
                marbleVX -= (1 + restitution_ground) * v_dot_n_ground * newNormalX;
                marbleVY -= (1 + restitution_ground) * v_dot_n_ground * newNormalY;
                marbleVZ -= (1 + restitution_ground) * v_dot_n_ground * newNormalZ;
            }
        } else {
            float oldY = marbleY;
            marbleY = fminf(marbleY, marbleY_before);
            if (marbleY > marbleY_before + 1e-4f) {
                printf("[DEBUG] Wall/corner collision tried to increase marbleY! oldY=%.4f, before=%.4f\n", marbleY, marbleY_before);
            }
            float reflection_nx_final = newNormalX;
            float reflection_ny_final = 0.0f;
            float reflection_nz_final = newNormalZ;
            float v_dot_n_wall = marbleVX * reflection_nx_final + 
                                 marbleVY * reflection_ny_final + 
                                 marbleVZ * reflection_nz_final;
            if (v_dot_n_wall < 0) {
                float oldVY = marbleVY;
                marbleVX -= (1 + restitution_wall) * v_dot_n_wall * reflection_nx_final;
                marbleVZ -= (1 + restitution_wall) * v_dot_n_wall * reflection_nz_final;
                if (marbleVY > oldVY + 1e-4f) {
                    printf("[DEBUG] Wall/corner collision tried to increase marbleVY! oldVY=%.4f, newVY=%.4f\n", oldVY, marbleVY);
                }
            }
        }
    }

    if (fabs(marbleVX) < 0.005f && fabs(accX) < 0.005f) marbleVX = 0.0f; // accX here is from start of frame
    if (fabs(marbleVZ) < 0.005f && fabs(accZ) < 0.005f) marbleVZ = 0.0f; // accZ here is from start of frame
}
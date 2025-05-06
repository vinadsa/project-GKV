#include "arena.h"
#include "globals.h" // Sekarang menyertakan GRID_SIZE, BOUNDS, dll.
#include "utils.h"   // Untuk clamp
#include <cmath>     // Untuk floor, sqrt
#include <GL/glut.h>

// Definisi array global arenaHeights (GRID_SIZE sekarang dari globals.h)
float arenaHeights[GRID_SIZE][GRID_SIZE]; // Definisi


// --- Arena Building Helper Functions ---
void addFlatArea(float centerX, float centerZ, float width, float depth, float height) {
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    int start_i = floor((centerX - width / 2.0f + BOUNDS) / stepX);
    int end_i = floor((centerX + width / 2.0f + BOUNDS) / stepX);
    int start_j = floor((centerZ - depth / 2.0f + BOUNDS) / stepZ);
    int end_j = floor((centerZ + depth / 2.0f + BOUNDS) / stepZ);

    start_i = clamp(start_i, 0, GRID_SIZE - 1);
    end_i = clamp(end_i, 0, GRID_SIZE - 1);
    start_j = clamp(start_j, 0, GRID_SIZE - 1);
    end_j = clamp(end_j, 0, GRID_SIZE - 1);

    for (int i = start_i; i <= end_i; ++i) {
        for (int j = start_j; j <= end_j; ++j) {
            arenaHeights[i][j] = height;
        }
    }
}

void addRampArea(float centerX, float centerZ, float width, float depth, float startHeight, float endHeight, char axis) {
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    int start_i = floor((centerX - width / 2.0f + BOUNDS) / stepX);
    int end_i = floor((centerX + width / 2.0f + BOUNDS) / stepX);
    int start_j = floor((centerZ - depth / 2.0f + BOUNDS) / stepZ);
    int end_j = floor((centerZ + depth / 2.0f + BOUNDS) / stepZ);

    start_i = clamp(start_i, 0, GRID_SIZE - 1);
    end_i = clamp(end_i, 0, GRID_SIZE - 1);
    start_j = clamp(start_j, 0, GRID_SIZE - 1);
    end_j = clamp(end_j, 0, GRID_SIZE - 1);

    for (int i = start_i; i <= end_i; ++i) {
        for (int j = start_j; j <= end_j; ++j) {
            float current_x_world = -BOUNDS + (float)i * stepX;
            float current_z_world = -BOUNDS + (float)j * stepZ;
            float progress = 0.0f;

            if (axis == 'x') {
                progress = (current_x_world - (centerX - width / 2.0f)) / width;
            } else {
                progress = (current_z_world - (centerZ - depth / 2.0f)) / depth;
            }
            progress = clamp(progress, 0.0f, 1.0f);
            arenaHeights[i][j] = startHeight + progress * (endHeight - startHeight);
        }
    }
}

void setupArenaGeometry() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            arenaHeights[i][j] = defaultFallingHeight;
        }
    }

    addFlatArea(0.0f, -BOUNDS + 2.0f, 5.0f, 4.0f, pathBaseHeight);
    addFlatArea(0.0f, -BOUNDS + 7.0f, 5.0f, 6.0f, pathBaseHeight);
    addRampArea(0.0f, -BOUNDS + 12.0f, 5.0f, 4.0f, pathBaseHeight, pathBaseHeight + 2.0f, 'z');
    addFlatArea(0.0f, -BOUNDS + 16.0f, 5.0f, 4.0f, pathBaseHeight + 2.0f);
    addFlatArea(-4.0f, -BOUNDS + 16.0f, 8.0f, 5.0f, pathBaseHeight + 2.0f);
    addFlatArea(-BOUNDS + 4.0f, -BOUNDS + 16.0f, 8.0f, 5.0f, pathBaseHeight + 2.0f);
    addRampArea(-BOUNDS + 8.0f, -BOUNDS + 12.0f, 4.0f, 5.0f, pathBaseHeight + 2.0f, pathBaseHeight, 'x');
    addFlatArea(-BOUNDS + 12.0f, -BOUNDS + 12.0f, 4.0f, 5.0f, pathBaseHeight);
}

// --- Arena Physics Lookup Functions ---
float getArenaHeight(float x, float z) {
    float clampedX = clamp(x, -BOUNDS, BOUNDS);
    float clampedZ = clamp(z, -BOUNDS, BOUNDS);

    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    float grid_x_float = (clampedX + BOUNDS) / stepX;
    float grid_z_float = (clampedZ + BOUNDS) / stepZ;

    int i = floor(grid_x_float);
    int j = floor(grid_z_float);

    i = clamp(i, 0, GRID_SIZE - 2);
    j = clamp(j, 0, GRID_SIZE - 2);

    float u = grid_x_float - i;
    float v = grid_z_float - j;

    float h00 = arenaHeights[i][j];
    float h10 = arenaHeights[i + 1][j];
    float h01 = arenaHeights[i][j + 1];
    float h11 = arenaHeights[i + 1][j + 1];

    float h_bottom = h00 * (1.0f - u) + h10 * u;
    float h_top = h01 * (1.0f - u) + h11 * u;
    return h_bottom * (1.0f - v) + h_top * v;
}

void getArenaHeightAndNormal(float x, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ) {
    outHeight = getArenaHeight(x, z);

    const float epsilon = 0.05f;
    float h_center = outHeight;
    float h_dx = getArenaHeight(x + epsilon, z);
    float h_dz = getArenaHeight(x, z + epsilon);

    outNormalX = (h_dx - h_center) * epsilon - 0.0f;
    outNormalY = 0.0f - epsilon * epsilon;
    outNormalZ = epsilon * (h_dz - h_center) - 0.0f;

    float normalLength = sqrt(outNormalX * outNormalX + outNormalY * outNormalY + outNormalZ * outNormalZ);
    if (normalLength > 1e-6) {
        outNormalX /= normalLength;
        outNormalY /= normalLength;
        outNormalZ /= normalLength;
    } else {
        outNormalX = 0.0f;
        outNormalY = 1.0f;
        outNormalZ = 0.0f;
    }

    if (outNormalY < 0) {
        outNormalX *= -1;
        outNormalY *= -1;
        outNormalZ *= -1;
    }
}

// --- Drawing Functions ---
void drawGround() {
    glColor3f(0.2f, 0.7f, 0.2f);

    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    glBegin(GL_QUADS);
    for (int i = 0; i < GRID_SIZE - 1; ++i) {
        for (int j = 0; j < GRID_SIZE - 1; ++j) {
            float x1 = -BOUNDS + (float)i * stepX;
            float z1 = -BOUNDS + (float)j * stepZ;
            float x2 = -BOUNDS + (float)(i + 1) * stepX;
            float z2 = -BOUNDS + (float)(j + 1) * stepZ;

            float y11 = arenaHeights[i][j];
            float y21 = arenaHeights[i + 1][j];
            float y22 = arenaHeights[i + 1][j + 1];
            float y12 = arenaHeights[i][j + 1];

            float avg_height = (y11 + y12 + y21 + y22) / 4.0f;
            float lowest_viz_height = defaultFallingHeight;
            float highest_viz_height = pathBaseHeight + 4.0f;
            float height_range = highest_viz_height - lowest_viz_height;
            float color_factor = clamp((avg_height - lowest_viz_height) / height_range, 0.0f, 1.0f);

            glColor3f(
                0.4f + color_factor * 0.2f,
                0.3f + color_factor * 0.4f,
                0.2f + color_factor * 0.2f
            );

            glVertex3f(x1, y11, z1);
            glVertex3f(x1, y12, z2);
            glVertex3f(x2, y22, z2);
            glVertex3f(x2, y21, z1);
        }
    }
    glEnd();
}
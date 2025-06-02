#include "arena.h"
#include "globals.h" // Sekarang menyertakan GRID_SIZE, BOUNDS, defaultFallingHeight, pathBaseHeight, dll.
#include "utils.h"   // Untuk clamp
#include <cmath>     // Untuk floor, sqrt
#include <GL/glut.h>
#include <cstdio>    // Untuk printf
#include <vector>    // Untuk std::vector

// --- Define arenaHeights global array here ---
// float arenaHeights[GRID_SIZE][GRID_SIZE] = {0};

// Define properties for the test cube and ramp
// These values match what was previously hardcoded in drawGround
static const float testCubeX = 0.0f;
static const float testCubeY = 2.0f;      // Center Y
static const float testCubeZ = 5.0f;
static const float testCubeSizeX = 2.0f;
static const float testCubeSizeY = 2.0f;  // Total height
static const float testCubeSizeZ = 2.0f;

static const float testRampX = 0.0f;
static const float testRampY = 1.0f;      // Center Y
static const float testRampZ = 2.5f;      // Center Z
static const float testRampSizeX = 2.0f;  // Width for Z-ramp, Length for X-ramp
static const float testRampSizeY = 2.0f;  // Total height difference of the slope
static const float testRampSizeZ = 3.0f;  // Length for Z-ramp, Width for X-ramp
static const char testRampAxis = 'z';

// Definisi array global arenaHeights (GRID_SIZE sekarang dari globals.h)
float arenaHeights[GRID_SIZE][GRID_SIZE] = {0};

// --- Arena Object Structs and Arrays ---
struct ArenaCube {
    float x, y, z, sizeX, sizeY, sizeZ;
};
struct ArenaRamp {
    float x, y, z, sizeX, sizeY, sizeZ; char axis;
};
struct ArenaBush {
    float x, y, z, radius;
};
struct ArenaTree {
    float x, y, z;
    float trunkHeight, trunkRadius;
    float foliageRadius;
};
struct ArenaRock {
    float x, y, z;
    float scale;
};

std::vector<ArenaCube> cubes;
std::vector<ArenaRamp> ramps;
std::vector<ArenaBush> bushes;
std::vector<ArenaTree> trees;
std::vector<ArenaRock> rocks;

void CreateCube(float x, float y, float z, float sizeX, float sizeY, float sizeZ) {
    cubes.push_back({x, y, z, sizeX, sizeY, sizeZ});
    // Update collision grid for physics
    // Assume ground is at y=0, so only raise height if cube is above
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    int start_i = floor((x - sizeX / 2.0f + BOUNDS) / stepX);
    int end_i   = floor((x + sizeX / 2.0f + BOUNDS) / stepX);
    int start_j = floor((z - sizeZ / 2.0f + BOUNDS) / stepZ);
    int end_j   = floor((z + sizeZ / 2.0f + BOUNDS) / stepZ);
    start_i = clamp(start_i, 0, GRID_SIZE - 1);
    end_i   = clamp(end_i,   0, GRID_SIZE - 1);
    start_j = clamp(start_j, 0, GRID_SIZE - 1);
    end_j   = clamp(end_j,   0, GRID_SIZE - 1);
    float topSurfaceHeight = y + sizeY / 2.0f;
    for (int i = start_i; i <= end_i; ++i) {
        for (int j = start_j; j <= end_j; ++j) {
            if (arenaHeights[i][j] < topSurfaceHeight) {
                arenaHeights[i][j] = topSurfaceHeight;
            }
        }
    }
}
void CreateRamp(float x, float y, float z, float sizeX, float sizeY, float sizeZ, char axis) {
    ramps.push_back({x, y, z, sizeX, sizeY, sizeZ, axis});
    // Update collision grid for physics
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    int start_i = floor((x - (axis=='x'?sizeX:sizeX)/2.0f + BOUNDS) / stepX);
    int end_i   = floor((x + (axis=='x'?sizeX:sizeX)/2.0f + BOUNDS) / stepX);
    int start_j = floor((z - (axis=='x'?sizeZ:sizeZ)/2.0f + BOUNDS) / stepZ);
    int end_j   = floor((z + (axis=='x'?sizeZ:sizeZ)/2.0f + BOUNDS) / stepZ);
    start_i = clamp(start_i, 0, GRID_SIZE - 1);
    end_i   = clamp(end_i,   0, GRID_SIZE - 1);
    start_j = clamp(start_j, 0, GRID_SIZE - 1);
    end_j   = clamp(end_j,   0, GRID_SIZE - 1);
    for (int i = start_i; i <= end_i; ++i) {
        for (int j = start_j; j <= end_j; ++j) {
            float world_x = -BOUNDS + i * stepX;
            float world_z = -BOUNDS + j * stepZ;
            float progress = 0.0f;
            if (axis == 'x') {
                progress = (world_x - (x - sizeX/2.0f)) / sizeX;
            } else {
                progress = (world_z - (z - sizeZ/2.0f)) / sizeZ;
            }
            progress = clamp(progress, 0.0f, 1.0f);
            float rampBaseY = y - sizeY/2.0f;
            float rampHeight = rampBaseY + progress * sizeY;
            if (arenaHeights[i][j] < rampHeight) {
                arenaHeights[i][j] = rampHeight;
            }
        }
    }
}

void CreateBush(float x, float y, float z, float radius) {
    bushes.push_back({x, y, z, radius});
}

void CreateTree(float x, float y, float z, float trunkHeight, float trunkRadius, float foliageRadius) {
    trees.push_back({x, y, z, trunkHeight, trunkRadius, foliageRadius});
}

void CreateRock(float x, float y, float z, float scale) {
    rocks.push_back({x, y, z, scale});
}

// --- Arena Building Helper Functions ---
void drawCube(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ) {
    // Draw shadow for the cube (planar shadow, similar to marble)
    if (enableShadows) {
        glPushMatrix();
            // Set up robust shadow projection matrix (same as marble)
            GLfloat shadow_plane[4] = {0.0f, 1.0f, 0.0f, -0.01f}; // y=0.01 plane
            GLfloat shadow_light[4] = {10.0f, 80.0f, 10.0f, 1.0f};
            extern void glShadowProjection(const float*, const float*); // Use from graphics.cpp
            glShadowProjection(shadow_light, shadow_plane);
            glTranslatef(centerX, centerY, centerZ);
            glScalef(sizeX, sizeY, sizeZ);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-2.0f, -2.0f);
            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glutSolidCube(1.0);
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
            glDisable(GL_POLYGON_OFFSET_FILL);
        glPopMatrix();
    }

    // Draw the actual cube (restore all OpenGL state before this)
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(sizeX, sizeY, sizeZ);
    // Restore color and material for the cube itself
    glColor3f(0.7f, 0.6f, 0.5f); // or use the color/material set by the caller
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawRamp(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ, char axis) {
    // Draw shadow for the ramp FIRST (before actual ramp)
    if (enableShadows) {
        glPushMatrix();
            // Set up robust shadow projection matrix (same as marble)
            GLfloat shadow_plane[4] = {0.0f, 1.0f, 0.0f, -0.01f};
            GLfloat shadow_light[4] = {10.0f, 80.0f, 10.0f, 1.0f};
            extern void glShadowProjection(const float*, const float*);
            glShadowProjection(shadow_light, shadow_plane);
            glTranslatef(centerX, centerY, centerZ);
            
            // Setengah ukuran untuk kemudahan perhitungan vertex
            float hx = sizeX / 2.0f;
            float hy = sizeY / 2.0f; 
            float hz = sizeZ / 2.0f;
              glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-2.0f, -2.0f);
            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D); // Disable any textures
            glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // Pure black shadow
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Simplified shadow geometry - just main surfaces
            if (axis == 'z') {
                float v[6][3] = {
                    {-hx, -hy, -hz}, { hx, -hy, -hz}, {-hx, -hy,  hz}, 
                    { hx, -hy,  hz}, {-hx,  hy,  hz}, { hx,  hy,  hz}
                };
                // Draw sloped surface shadow
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
                glEnd();
                // Draw bottom surface shadow
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[2]); glVertex3fv(v[3]); glVertex3fv(v[1]);
                glEnd();
            } else if (axis == 'x') {
                float v[6][3] = {
                    {-hx, -hy, -hz}, {-hx, -hy,  hz}, { hx, -hy, -hz}, 
                    { hx, -hy,  hz}, { hx,  hy, -hz}, { hx,  hy,  hz}
                };
                // Draw sloped surface shadow
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
                glEnd();
                // Draw bottom surface shadow
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[2]); glVertex3fv(v[3]); glVertex3fv(v[1]);
                glEnd();
            }
              glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
            glDisable(GL_POLYGON_OFFSET_FILL);
        glPopMatrix();
    }

    // Now draw the actual ramp (with proper material colors)
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ); // Pindahkan ke pusat ramp
    
    // Restore proper color/material after shadow rendering
    glColor3f(0.7f, 0.6f, 0.5f); // Ramp color (similar to cube)

    // Setengah ukuran untuk kemudahan perhitungan vertex
    // sizeY adalah tinggi total ramp. Puncak ramp akan di centerY + sizeY/2, dasar di centerY - sizeY/2
    float hx = sizeX / 2.0f;
    float hy = sizeY / 2.0f; 
    float hz = sizeZ / 2.0f;

    if (axis == 'z') { // Ramp miring sepanjang sumbu Z
        
        float v[6][3] = {
            {-hx, -hy, -hz}, // v0 (depan-bawah-kiri, kaki ramp)
            { hx, -hy, -hz}, // v1 (depan-bawah-kanan, kaki ramp)
            {-hx, -hy,  hz}, // v2 (belakang-bawah-kiri, di dasar bagian vertikal)
            { hx, -hy,  hz}, // v3 (belakang-bawah-kanan, di dasar bagian vertikal)
            {-hx,  hy,  hz}, // v4 (belakang-atas-kiri, puncak ramp)
            { hx,  hy,  hz}  // v5 (belakang-atas-kanan, puncak ramp)
        };

        
        float norm_slope_x = 0;
        float norm_slope_y = -sizeX * sizeZ; // Ini akan menjadi komponen Y dari normal jika ramp horizontal
                                          // Untuk ramp miring, ini adalah komponen yang tegak lurus ke XZ plane
        float norm_slope_z = sizeX * sizeY;
        float len_slope = sqrt(norm_slope_x*norm_slope_x + norm_slope_y*norm_slope_y + norm_slope_z*norm_slope_z);
        if (len_slope > 1e-6) {
            norm_slope_x /= len_slope;
            norm_slope_y /= len_slope;
            norm_slope_z /= len_slope;
        }
      
        norm_slope_x = 0;
        norm_slope_y = sizeX * sizeZ;
        norm_slope_z = -sizeX * sizeY;
        len_slope = sqrt(norm_slope_y*norm_slope_y + norm_slope_z*norm_slope_z); // norm_slope_x is 0
        if (len_slope > 1e-6) {
            norm_slope_y /= len_slope;
            norm_slope_z /= len_slope;
        }


        // 1. Permukaan miring (Top/Sloped face) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(norm_slope_x, norm_slope_y, norm_slope_z);
        glVertex3fv(v[0]); // depan-bawah-kiri (kaki ramp)
        glVertex3fv(v[1]); // depan-bawah-kanan (kaki ramp)
        glVertex3fv(v[5]); // belakang-atas-kanan (puncak ramp)
        glVertex3fv(v[4]); // belakang-atas-kiri (puncak ramp)
        glEnd();

        // 2. Alas (Bottom face) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f); // Menghadap ke bawah
        glVertex3fv(v[0]); // depan-bawah-kiri
        glVertex3fv(v[2]); // belakang-bawah-kiri
        glVertex3fv(v[3]); // belakang-bawah-kanan
        glVertex3fv(v[1]); // depan-bawah-kanan
        glEnd();

        // 3. Sisi belakang vertikal (Back face, di z = hz) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f); // Menghadap ke +Z
        glVertex3fv(v[2]); // belakang-bawah-kiri
        glVertex3fv(v[3]); // belakang-bawah-kanan
        glVertex3fv(v[5]); // belakang-atas-kanan
        glVertex3fv(v[4]); // belakang-atas-kiri
        glEnd();

        // 4. Sisi kiri (Left triangle) - TRIANGLE
        glBegin(GL_TRIANGLES);
        glNormal3f(-1.0f, 0.0f, 0.0f); // Menghadap ke -X
        glVertex3fv(v[0]); // depan-bawah-kiri
        glVertex3fv(v[4]); // belakang-atas-kiri
        glVertex3fv(v[2]); // belakang-bawah-kiri
        glEnd();

        // 5. Sisi kanan (Right triangle) - TRIANGLE
        glBegin(GL_TRIANGLES);
        glNormal3f(1.0f, 0.0f, 0.0f); // Menghadap ke +X
        glVertex3fv(v[1]); // depan-bawah-kanan
        glVertex3fv(v[3]); // belakang-bawah-kanan
        glVertex3fv(v[5]); // belakang-atas-kanan
        glEnd();

    } else if (axis == 'x') { // Ramp miring sepanjang sumbu X
       
        float v[6][3] = {
            {-hx, -hy, -hz}, // v0 (kiri-bawah-depan, kaki ramp)
            {-hx, -hy,  hz}, // v1 (kiri-bawah-belakang, kaki ramp)
            { hx, -hy, -hz}, // v2 (kanan-bawah-depan, di dasar bagian vertikal)
            { hx, -hy,  hz}, // v3 (kanan-bawah-belakang, di dasar bagian vertikal)
            { hx,  hy, -hz}, // v4 (kanan-atas-depan, puncak ramp)
            { hx,  hy,  hz}  // v5 (kanan-atas-belakang, puncak ramp)
        };

        
        float norm_slope_x = sizeZ * sizeY;
        float norm_slope_y = -sizeZ * sizeX;
        float norm_slope_z = 0;
        float len_slope = sqrt(norm_slope_x*norm_slope_x + norm_slope_y*norm_slope_y); // norm_slope_z is 0
        if (len_slope > 1e-6) {
            norm_slope_x /= len_slope;
            norm_slope_y /= len_slope;
        }
        // Balik normal jika Y negatif (agar mengarah ke atas dari permukaan)
        if (norm_slope_y < 0) {
            norm_slope_x *= -1;
            norm_slope_y *= -1;
        }


        // 1. Permukaan miring (Top/Sloped face) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(norm_slope_x, norm_slope_y, norm_slope_z);
        glVertex3fv(v[0]); // kiri-bawah-depan (kaki ramp)
        glVertex3fv(v[1]); // kiri-bawah-belakang (kaki ramp)
        glVertex3fv(v[5]); // kanan-atas-belakang (puncak ramp)
        glVertex3fv(v[4]); // kanan-atas-depan (puncak ramp)
        glEnd();

        // 2. Alas (Bottom face) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f); // Menghadap ke bawah
        glVertex3fv(v[0]); // kiri-bawah-depan
        glVertex3fv(v[2]); // kanan-bawah-depan
        glVertex3fv(v[3]); // kanan-bawah-belakang
        glVertex3fv(v[1]); // kiri-bawah-belakang
        glEnd();

        // 3. Sisi "belakang" vertikal (di x = hx) (Right face) - QUAD
        glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f); // Menghadap ke +X
        glVertex3fv(v[2]); // kanan-bawah-depan
        glVertex3fv(v[3]); // kanan-bawah-belakang
        glVertex3fv(v[5]); // kanan-atas-belakang
        glVertex3fv(v[4]); // kanan-atas-depan
        glEnd();

        // 4. Sisi "depan" (di -Z) (Front triangle) - TRIANGLE
        glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, 0.0f, -1.0f); // Menghadap ke -Z
        glVertex3fv(v[0]); // kiri-bawah-depan
        glVertex3fv(v[4]); // kanan-atas-depan
        glVertex3fv(v[2]); // kanan-bawah-depan
        glEnd();

        // 5. Sisi "belakang" (di +Z) (Back triangle) - TRIANGLE
        glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, 0.0f, 1.0f); // Menghadap ke +Z
        glVertex3fv(v[1]); // kiri-bawah-belakang
        glVertex3fv(v[3]); // kanan-bawah-belakang
        glVertex3fv(v[5]); // kanan-atas-belakang
        glEnd();
    }    glPopMatrix();
}


void setupArenaGeometry() {
    cubes.clear(); ramps.clear(); bushes.clear(); trees.clear();
    CreateRamp(1.0f, 1.0f, -3.0f, 2.0f, 2.0f, 1.0f, 'x'); // Ramp ke arah X
    CreateCube(7.0f, 2.0f, -3.0f, 10.0f, 1.0f, 1.0f); // Cube di kanan
    CreateCube(10.0f, 2.0f, -3.0f, 1.0f, 1.0f, 10.0f); // Cube di kanan
    CreateRamp(10.0f, 2.7f, 1.25f, 2.0f, 2.0f, 7.0f, 'z');
    CreateCube(10.17f, 0.50f, 6.09f, 1.0f, 1.0f, 1.0f);          // Cube baru
    CreateRamp(10.17f, 0.50f, 6.09f, 1.0f, 1.0f, 1.0f, 'z');     // Ramp arah Z
 
    CreateCube(10.17f, 0.5f, 9.59f, 1.0f, 1.0f, 6.0f);
    CreateRamp(10.17f, 2.0f, 14.59f, 1.0f, 2.0f, 4.0f, 'z');
    CreateCube(10.17f, 2.5f, 20.09f, 3.0f, 1.0f, 7.0f);

    CreateRamp(13.67f, 3.75f, 20.09f, 4.0f, 1.5f, 2.0f, 'x');
    CreateCube(18.17f, 4.0f, 20.09f, 5.0f, 1.0f, 2.0f);

    CreateRamp(23.17f, 3.25f, 20.09f, 5.0f, 2.5f, 2.0f, 'x');
    CreateCube(28.67f, 1.5f, 20.09f, 6.0f, 1.0f, 4.0f);
    CreateCube(28.67f, 1.5f, 23.59f, 1.0f, 1.0f, 1.0f);
    CreateCube(28.67f, 1.5f, 25.59f, 1.0f, 1.0f, 1.0f);
    CreateCube(28.67f, 1.5f, 27.59f, 1.0f, 1.0f, 1.0f);
    CreateRamp(28.67f, 3.5f, 31.09f, 1.0f, 3.0f, 6.0f, 'z');    CreateCube(28.67f, 4.5f, 38.09f, 8.0f, 1.0f, 8.0f);

    // Create decorative bushes (semak-semak)
    CreateBush(-5.0f, 0.5f, -10.0f, 0.8f);   // Bush near start area
    CreateBush(-3.0f, 0.4f, -8.0f, 0.6f);    // Smaller bush
    CreateBush(5.0f, 0.7f, 0.0f, 0.7f);      // Medium bush near middle
    CreateBush(15.0f, 0.6f, 15.0f, 0.7f);    // Bush near platform area
    CreateBush(25.0f, 0.5f, 10.0f, 0.7f);    // Bush near end area
    CreateBush(-10.50f, 0.50f, 13.14f, 0.5f);
    CreateBush(30.0f, 0.4f, 30.0f, 0.5f);    // Small bush near final area
    CreateBush(-1.44f, 0.50f, 28.83f, 0.7f);
    CreateBush(20.71f, 0.50f, -12.45f, 0.7f);

    
    CreateTree(21.71f, 0.30f, -10.45f, 10.0f, 0.2f, 1.5f); // Another tree
    CreateTree(6.71f, 0.30f, -10.45f, 7.0f, 0.2f, 1.5f); // Another tree
    CreateTree(15.71f, 0.30f, 9.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(-5.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(-7.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(-10.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(-11.71f, 0.30f, -14.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(-13.71f, 0.30f, -14.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(14.71f, 0.30f, -21.45f, 5.0f, 0.2f, 1.5f); // Another tree
    CreateTree(14.71f, 0.30f, -27.45f, 5.0f, 0.2f, 1.5f); // Another tree

    // Add some rocks to the scene for decoration
    CreateRock(18.0f, 0.0f, 12.0f, 1.2f);    // Large rock near trees
    CreateRock(-8.0f, 0.0f, -20.0f, 0.8f);   // Medium rock 
    CreateRock(25.0f, 0.0f, -15.0f, 1.5f);   // Large rock
    CreateRock(-12.0f, 0.0f, 15.0f, 0.6f);   // Small rock
    CreateRock(10.0f, 0.0f, 25.0f, 1.0f);    // Medium rock near end area
    CreateRock(-25.0f, 0.0f, 5.0f, 1.3f);    // Large rock on the side
    CreateRock(5.0f, 0.0f, -25.0f, 0.9f);    // Medium rock

    // Developer bisa tambah sendiri: CreateCube(...), CreateRamp(...), CreateBush(...), CreateRock(...)
}


// --- Arena Physics Lookup Functions ---
float getArenaHeight(float x, float z) {
    float height = 0.0f; // Ground at Y=0
    // Check all cubes
    for (const auto& c : cubes) {
        float minX = c.x - c.sizeX / 2.0f;
        float maxX = c.x + c.sizeX / 2.0f;
        float minZ = c.z - c.sizeZ / 2.0f;
        float maxZ = c.z + c.sizeZ / 2.0f;
        float topY = c.y + c.sizeY / 2.0f;
        if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            if (topY > height) height = topY;
        }
    }
    // Check all ramps
    for (const auto& r : ramps) {
        if (r.axis == 'z') {
            float minX = r.x - r.sizeX / 2.0f;
            float maxX = r.x + r.sizeX / 2.0f;
            float minZ = r.z - r.sizeZ / 2.0f;
            float maxZ = r.z + r.sizeZ / 2.0f;
            if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
                float progress = (z - minZ) / r.sizeZ;
                progress = clamp(progress, 0.0f, 1.0f);
                float baseY = r.y - r.sizeY / 2.0f;
                float rampY = baseY + progress * r.sizeY;
                if (rampY > height) height = rampY;
            }
        } else if (r.axis == 'x') {
            float minZ = r.z - r.sizeZ / 2.0f;
            float maxZ = r.z + r.sizeZ / 2.0f;
            float minX = r.x - r.sizeX / 2.0f;
            float maxX = r.x + r.sizeX / 2.0f;
            if (z >= minZ && z <= maxZ && x >= minX && x <= maxX) {
                float progress = (x - minX) / r.sizeX;
                progress = clamp(progress, 0.0f, 1.0f);
                float baseY = r.y - r.sizeY / 2.0f;
                float rampY = baseY + progress * r.sizeY;
                if (rampY > height) height = rampY;
            }
        }
    }
    return height;
}

void getArenaHeightAndNormal(float x, float z, float& height, float& outNormalX, float& outNormalY, float& outNormalZ) {
    // Initialize with ground plane
    height = 0.0f;
    outNormalX = 0.0f; outNormalY = 1.0f; outNormalZ = 0.0f;
    const float epsilon = 0.015f;
    const float epsilon_normal_Y_diff = 0.05f;
    const float wall_like_threshold_Y = 0.5f;
    float current_best_h = height;
    float current_best_nx = outNormalX, current_best_ny = outNormalY, current_best_nz = outNormalZ;
    // Check all cubes
    for (const auto& c : cubes) {
        float minX = c.x - c.sizeX / 2.0f;
        float maxX = c.x + c.sizeX / 2.0f;
        float minZ = c.z - c.sizeZ / 2.0f;
        float maxZ = c.z + c.sizeZ / 2.0f;
        float topY = c.y + c.sizeY / 2.0f;
        float obj_h = -1.0f, obj_nx = 0.0f, obj_ny = 0.0f, obj_nz = 0.0f;
        bool obj_is_wall = false;
        bool on_x_min = fabs(x - minX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon);
        bool on_x_max = fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon);
        bool on_z_min = fabs(z - minZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon);
        bool on_z_max = fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon);
        int wall_count = (on_x_min ? 1 : 0) + (on_x_max ? 1 : 0) + (on_z_min ? 1 : 0) + (on_z_max ? 1 : 0);
        if (wall_count >= 2) {
            obj_nx = (on_x_min ? -1.0f : 0.0f) + (on_x_max ? 1.0f : 0.0f);
            obj_nz = (on_z_min ? -1.0f : 0.0f) + (on_z_max ? 1.0f : 0.0f);
            float len = sqrt(obj_nx*obj_nx + obj_nz*obj_nz);
            if (len > 1e-6) { obj_nx /= len; obj_nz /= len; }
            obj_is_wall = true;
            obj_h = topY;
            obj_ny = 0.0f;
        } else if (on_x_min) { obj_nx = -1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_x_max) { obj_nx =  1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_z_min) { obj_nz = -1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_z_max) { obj_nz =  1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            obj_h = topY;
            obj_ny = 1.0f; obj_nx = 0.0f; obj_nz = 0.0f; obj_is_wall = false;
        } else {
            obj_h = -1.0f;
        }
        if (obj_h > -0.5f) {
            if (obj_h > current_best_h + epsilon) {
                current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
            } else if (fabs(obj_h - current_best_h) < epsilon) {
                bool current_is_wall_type = (current_best_ny < wall_like_threshold_Y && current_best_ny > -wall_like_threshold_Y);
                bool new_obj_is_wall_type = (obj_ny < wall_like_threshold_Y && obj_ny > -wall_like_threshold_Y);
                if (!new_obj_is_wall_type && current_is_wall_type) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (new_obj_is_wall_type && !current_is_wall_type) {
                    ;
                } else if (new_obj_is_wall_type && current_is_wall_type) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (!new_obj_is_wall_type && !current_is_wall_type) {
                    if (obj_ny > current_best_ny + epsilon_normal_Y_diff) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    }
                }
            }
        }
    }
    // Check all ramps
    for (const auto& r : ramps) {
        float baseY = r.y - r.sizeY / 2.0f;
        float topY = r.y + r.sizeY / 2.0f;
        float minX = r.x - r.sizeX / 2.0f;
        float maxX = r.x + r.sizeX / 2.0f;
        float minZ = r.z - r.sizeZ / 2.0f;
        float maxZ = r.z + r.sizeZ / 2.0f;
        bool broad_phase_hit = false;
        if (r.axis == 'z') {
            broad_phase_hit = (x >= minX - epsilon && x <= maxX + epsilon && z >= minZ - epsilon && z <= maxZ + epsilon);
        } else {
            broad_phase_hit = (z >= minZ - epsilon && z <= maxZ + epsilon && x >= minX - epsilon && x <= maxX + epsilon);
        }
        if (broad_phase_hit) {
            float obj_h = -1.0f, obj_nx = 0.0f, obj_ny = 0.0f, obj_nz = 0.0f; bool obj_is_wall = false;
            bool on_strict_footprint = false;
            if (r.axis == 'z') {
                on_strict_footprint = (x >= minX && x <= maxX && z >= minZ && z <= maxZ);
            } else {
                on_strict_footprint = (z >= minZ && z <= maxZ && x >= minX && x <= maxX);
            }
            if (on_strict_footprint) {
                float progress = 0.0f;
                if (r.axis == 'z') {
                    progress = (z - minZ) / r.sizeZ;
                    obj_nx = 0; obj_ny = r.sizeZ; obj_nz = -r.sizeY;
                } else {
                    progress = (x - minX) / r.sizeX;
                    obj_nx = -r.sizeY; obj_ny = r.sizeX; obj_nz = 0;
                }
                obj_h = baseY + progress * r.sizeY;
                // No y check here in 2D version
                float len = sqrt(obj_nx*obj_nx + obj_ny*obj_ny + obj_nz*obj_nz);
                if (len > 1e-6) { obj_nx /= len; obj_ny /= len; obj_nz /= len; }
                if (obj_ny < 0) { obj_nx *= -1; obj_ny *= -1; obj_nz *= -1; }
                obj_is_wall = (obj_ny < wall_like_threshold_Y);
            } else {
                if (r.axis == 'z') {
                    if (fabs(x - minX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = -1.0f; obj_is_wall = true;
                    } else if (fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = 1.0f; obj_is_wall = true;
                    } else if (fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = 1.0f; obj_is_wall = true;
                    }
                } else {
                    if (fabs(z - minZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = -1.0f; obj_is_wall = true;
                    } else if (fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = 1.0f; obj_is_wall = true;
                    } else if (fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = 1.0f; obj_is_wall = true;
                    }
                }
                if (obj_is_wall) {
                    obj_h = topY; obj_ny = 0.0f;
                } else {
                    obj_h = -1.0f;
                }
            }
            if (obj_h > -0.5f) {
                if (obj_h > current_best_h + epsilon) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (fabs(obj_h - current_best_h) < epsilon) {
                    bool current_is_wall_type = (current_best_ny < wall_like_threshold_Y && current_best_ny > -wall_like_threshold_Y);
                    bool new_obj_is_wall_type = (obj_ny < wall_like_threshold_Y && obj_ny > -wall_like_threshold_Y);
                    if (!new_obj_is_wall_type && current_is_wall_type) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    } else if (new_obj_is_wall_type && !current_is_wall_type) {
                        ;
                    } else if (new_obj_is_wall_type && current_is_wall_type) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    } else if (!new_obj_is_wall_type && !current_is_wall_type) {
                        if (obj_ny > current_best_ny + epsilon_normal_Y_diff) {
                            current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                        }
                    }
                }
            }
        }
    }
    height = current_best_h;
    outNormalX = current_best_nx; outNormalY = current_best_ny; outNormalZ = current_best_nz;
}

// --- 3D-aware Arena Physics Lookup Functions ---
// Only consider cubes/ramps for collision if marble is above their bottom surface
float getArenaHeightAt(float x, float y, float z) {
    float height = 0.0f; // Ground at Y=0
    const float epsilon = 0.01f;
    // Check all cubes
    for (const auto& c : cubes) {
        float minX = c.x - c.sizeX / 2.0f;
        float maxX = c.x + c.sizeX / 2.0f;
        float minZ = c.z - c.sizeZ / 2.0f;
        float maxZ = c.z + c.sizeZ / 2.0f;
        float topY = c.y + c.sizeY / 2.0f;
        float bottomY = c.y - c.sizeY / 2.0f;
        if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            // Only collide if marble is above or near the top
            if (y >= topY - epsilon) {
                if (topY > height) height = topY;
            }
        }
    }
    // Check all ramps
    for (const auto& r : ramps) {
        float minX = r.x - r.sizeX / 2.0f;
        float maxX = r.x + r.sizeX / 2.0f;
        float minZ = r.z - r.sizeZ / 2.0f;
        float maxZ = r.z + r.sizeZ / 2.0f;
        if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            float progress = (z - minZ) / r.sizeZ;
            progress = clamp(progress, 0.0f, 1.0f);
            float baseY = r.y - r.sizeY / 2.0f;
            float rampY = baseY + progress * r.sizeY;
            if (y >= rampY - epsilon) {
                if (rampY > height) height = rampY;
            }
        } else if (r.axis == 'x') {
            float minZ = r.z - r.sizeZ / 2.0f;
            float maxZ = r.z + r.sizeZ / 2.0f;
            float minX = r.x - r.sizeX / 2.0f;
            float maxX = r.x + r.sizeX / 2.0f;
            if (z >= minZ && z <= maxZ && x >= minX && x <= maxX) {
                float progress = (x - minX) / r.sizeX;
                progress = clamp(progress, 0.0f, 1.0f);
                float baseY = r.y - r.sizeY / 2.0f;
                float rampY = baseY + progress * r.sizeY;
                if (y >= rampY - epsilon) {
                    if (rampY > height) height = rampY;
                }
            }
        }
    }
    return height;
}

void getArenaHeightAndNormalAt(float x, float y, float z, float& height, float& outNormalX, float& outNormalY, float& outNormalZ) {
    // Like getArenaHeightAndNormal, but only consider surfaces below or at y
    height = 0.0f;
    outNormalX = 0.0f; outNormalY = 1.0f; outNormalZ = 0.0f;
    const float epsilon = 0.015f;
    const float epsilon_normal_Y_diff = 0.05f;
    const float wall_like_threshold_Y = 0.5f;
        float current_best_h = height;
    float current_best_nx = outNormalX, current_best_ny = outNormalY, current_best_nz = outNormalZ;
    
    // Check all cubes
    for (const auto& c : cubes) {
        float minX = c.x - c.sizeX / 2.0f;
        float maxX = c.x + c.sizeX / 2.0f;
        float minZ = c.z - c.sizeZ / 2.0f;
        float maxZ = c.z + c.sizeZ / 2.0f;
        float topY = c.y + c.sizeY / 2.0f;
        float bottomY = c.y - c.sizeY / 2.0f;
        float obj_h = -1.0f, obj_nx = 0.0f, obj_ny = 0.0f, obj_nz = 0.0f;
        bool obj_is_wall = false;
        bool on_x_min = fabs(x - minX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon);
        bool on_x_max = fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon);
        bool on_z_min = fabs(z - minZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon);
        bool on_z_max = fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon);
        int wall_count = (on_x_min ? 1 : 0) + (on_x_max ? 1 : 0) + (on_z_min ? 1 : 0) + (on_z_max ? 1 : 0);
        // Only block by wall if y is within the cube's vertical span
        if (wall_count >= 2 && y >= bottomY - epsilon && y <= topY + epsilon) {
            obj_nx = (on_x_min ? -1.0f : 0.0f) + (on_x_max ? 1.0f : 0.0f);
            obj_nz = (on_z_min ? -1.0f : 0.0f) + (on_z_max ? 1.0f : 0.0f);
            float len = sqrt(obj_nx*obj_nx + obj_nz*obj_nz);
            if (len > 1e-6) { obj_nx /= len; obj_nz /= len; }
            obj_is_wall = true;
            obj_h = topY;
            obj_ny = 0.0f;
        } else if (on_x_min && y >= bottomY - epsilon && y <= topY + epsilon) { obj_nx = -1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_x_max && y >= bottomY - epsilon && y <= topY + epsilon) { obj_nx =  1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_z_min && y >= bottomY - epsilon && y <= topY + epsilon) { obj_nz = -1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        else if (on_z_max && y >= bottomY - epsilon && y <= topY + epsilon) { obj_nz =  1.0f; obj_is_wall = true; obj_h = topY; obj_ny = 0.0f; }
        // Only filter by y for top face (not for walls)
        else if (x >= minX && x <= maxX && z >= minZ && z <= maxZ && y >= topY - epsilon) {
            obj_h = topY;
            obj_ny = 1.0f; obj_nx = 0.0f; obj_nz = 0.0f; obj_is_wall = false;
        } else {
            obj_h = -1.0f;
        }
        if (obj_h > -0.5f) {
            if (obj_h > current_best_h + epsilon) {
                current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
            } else if (fabs(obj_h - current_best_h) < epsilon) {
                bool current_is_wall_type = (current_best_ny < wall_like_threshold_Y && current_best_ny > -wall_like_threshold_Y);
                bool new_obj_is_wall_type = (obj_ny < wall_like_threshold_Y && obj_ny > -wall_like_threshold_Y);
                if (!new_obj_is_wall_type && current_is_wall_type) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (new_obj_is_wall_type && !current_is_wall_type) {
                    ;
                } else if (new_obj_is_wall_type && current_is_wall_type) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (!new_obj_is_wall_type && !current_is_wall_type) {
                    if (obj_ny > current_best_ny + epsilon_normal_Y_diff) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    }
                }
            }
        }
    }
    // Check all ramps
    for (const auto& r : ramps) {
        float baseY = r.y - r.sizeY / 2.0f;
        float topY = r.y + r.sizeY / 2.0f;
        float minX = r.x - r.sizeX / 2.0f;
        float maxX = r.x + r.sizeX / 2.0f;
        float minZ = r.z - r.sizeZ / 2.0f;
        float maxZ = r.z + r.sizeZ / 2.0f;
        bool broad_phase_hit = false;
        if (r.axis == 'z') {
            broad_phase_hit = (x >= minX - epsilon && x <= maxX + epsilon && z >= minZ - epsilon && z <= maxZ + epsilon);
        } else {
            broad_phase_hit = (z >= minZ - epsilon && z <= maxZ + epsilon && x >= minX - epsilon && x <= maxX + epsilon);
        }
        if (broad_phase_hit) {
            float obj_h = -1.0f, obj_nx = 0.0f, obj_ny = 0.0f, obj_nz = 0.0f; bool obj_is_wall = false;
            bool on_strict_footprint = false;
            if (r.axis == 'z') {
                on_strict_footprint = (x >= minX && x <= maxX && z >= minZ && z <= maxZ);
            } else {
                on_strict_footprint = (z >= minZ && z <= maxZ && x >= minX && x <= maxX);
            }
            if (on_strict_footprint) {
                float progress = 0.0f;
                if (r.axis == 'z') {
                    progress = (z - minZ) / r.sizeZ;
                    obj_nx = 0; obj_ny = r.sizeZ; obj_nz = -r.sizeY;
                } else {
                    progress = (x - minX) / r.sizeX;
                    obj_nx = -r.sizeY; obj_ny = r.sizeX; obj_nz = 0;
                }
                obj_h = baseY + progress * r.sizeY;
                // Add height check to prevent collision when marble is below ramp surface
                if (y >= obj_h - epsilon) {
                    float len = sqrt(obj_nx*obj_nx + obj_ny*obj_ny + obj_nz*obj_nz);
                    if (len > 1e-6) { obj_nx /= len; obj_ny /= len; obj_nz /= len; }
                    if (obj_ny < 0) { obj_nx *= -1; obj_ny *= -1; obj_nz *= -1; }
                    obj_is_wall = (obj_ny < wall_like_threshold_Y);
                } else {
                    // Marble is below ramp surface, no collision
                    obj_h = -1.0f;
                }
            } else {
                if (r.axis == 'z') {
                    if (fabs(x - minX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = -1.0f; obj_is_wall = true;
                    } else if (fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = 1.0f; obj_is_wall = true;
                    } else if (fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = 1.0f; obj_is_wall = true;
                    }
                } else {
                    if (fabs(z - minZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = -1.0f; obj_is_wall = true;
                    } else if (fabs(z - maxZ) < epsilon && (x >= minX - epsilon && x <= maxX + epsilon)) {
                        obj_nz = 1.0f; obj_is_wall = true;
                    } else if (fabs(x - maxX) < epsilon && (z >= minZ - epsilon && z <= maxZ + epsilon)) {
                        obj_nx = 1.0f; obj_is_wall = true;
                    }
                }
                if (obj_is_wall) {
                    obj_h = topY; obj_ny = 0.0f;
                } else {
                    obj_h = -1.0f;
                }
            }
            if (obj_h > -0.5f) {
                if (obj_h > current_best_h + epsilon) {
                    current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                } else if (fabs(obj_h - current_best_h) < epsilon) {
                    bool current_is_wall_type = (current_best_ny < wall_like_threshold_Y && current_best_ny > -wall_like_threshold_Y);
                    bool new_obj_is_wall_type = (obj_ny < wall_like_threshold_Y && obj_ny > -wall_like_threshold_Y);
                    if (!new_obj_is_wall_type && current_is_wall_type) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    } else if (new_obj_is_wall_type && !current_is_wall_type) {
                        ;
                    } else if (new_obj_is_wall_type && current_is_wall_type) {
                        current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                    } else if (!new_obj_is_wall_type && !current_is_wall_type) {
                        if (obj_ny > current_best_ny + epsilon_normal_Y_diff) {
                            current_best_h = obj_h; current_best_nx = obj_nx; current_best_ny = obj_ny; current_best_nz = obj_nz;
                        }
                    }
                }
            }
        }
    }
    height = current_best_h;
    outNormalX = current_best_nx; outNormalY = current_best_ny; outNormalZ = current_best_nz;
}


// --- Debug Helper: Print Marble Position for Object Placement ---
// Call this function from your input handler (e.g., when 'O' is pressed)
// Replace 'marble.x', 'marble.y', 'marble.z' with your actual marble variable names if different
void PrintMarblePositionForPlacement(float x, float y, float z) {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("CreateCube(%.2ff, %.2ff, %.2ff, sizeX, sizeY, sizeZ);\n", x, y, z);
    printf("CreateRamp(%.2ff, %.2ff, %.2ff, sizeX, sizeY, sizeZ, 'axis');\n", x, y, z);
    printf("CreateBush(%.2ff, %.2ff, %.2ff, radius);\n", x, y, z);
    printf("CreateTree(%.2ff, %.2ff, %.2ff, trunkHeight, trunkRadius, foliageRadius);\n", x, y, z);
    printf("CreateRock(%.2ff, %.2ff, %.2ff, scale);\n", x, y, z);
    printf("addCheckpoint(%.2ff, %.2ff, bonusMinutes);\n", x, z);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

// --- Drawing Functions ---
void drawGround() {
    // Set material properties for ground (flat plane) - Grass green color
    GLfloat ground_ambient[] = {0.1f, 0.4f, 0.1f, 1.0f};
    GLfloat ground_diffuse[] = {0.2f, 0.8f, 0.2f, 1.0f};
    GLfloat ground_specular[] = {0.1f, 0.2f, 0.1f, 1.0f};
    GLfloat ground_shininess = 8.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ground_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ground_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ground_shininess);
    glColor3f(0.2f, 0.8f, 0.2f); // Grass green color

    // Gambar ground sebagai sebuah quad besar
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); // Normal mengarah ke atas
    glVertex3f(-BOUNDS, 0.0f, -BOUNDS);
    glVertex3f( BOUNDS, 0.0f, -BOUNDS);
    glVertex3f( BOUNDS, 0.0f,  BOUNDS);
    glVertex3f(-BOUNDS, 0.0f,  BOUNDS);
    glEnd();

    // Material untuk kubus
    GLfloat cube_ambient[] = {0.5f, 0.4f, 0.3f, 1.0f};
    GLfloat cube_diffuse[] = {0.7f, 0.6f, 0.5f, 1.0f};
    GLfloat cube_specular[] = {0.2f, 0.2f, 0.1f, 1.0f};
    GLfloat cube_shininess = 10.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cube_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cube_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cube_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cube_shininess);
    glColor3f(0.7f, 0.6f, 0.5f); // Warna kubus
    for (const auto& c : cubes) {
        drawCube(c.x, c.y, c.z, c.sizeX, c.sizeY, c.sizeZ);
    }

    // Material untuk ramp
    GLfloat ramp_ambient[] = {0.3f, 0.3f, 0.5f, 1.0f};
    GLfloat ramp_diffuse[] = {0.5f, 0.5f, 0.7f, 1.0f};
    GLfloat ramp_specular[] = {0.1f, 0.1f, 0.2f, 1.0f};
    GLfloat ramp_shininess = 8.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ramp_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ramp_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ramp_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ramp_shininess);
    glColor3f(0.5f, 0.5f, 0.7f); // Warna ramp
    for (const auto& r : ramps) {
        drawRamp(r.x, r.y, r.z, r.sizeX, r.sizeY, r.sizeZ, r.axis);
    }

    // Draw bushes (decorative only)
    for (const auto& b : bushes) {
        drawBush(b.x, b.y, b.z, b.radius);
    }
    // Draw trees (decorative only)
    for (const auto& t : trees) {
        drawTree(t.x, t.y, t.z, t.trunkHeight, t.trunkRadius, t.foliageRadius);
    }
    
    // Draw rocks (decorative only)
    for (const auto& rock : rocks) {
        drawRock(rock.x, rock.y, rock.z, rock.scale);
    }
}


void drawTree(float x, float y, float z, float trunkHeight, float trunkRadius, float foliageRadius) {
    // Set material properties for trunk (bark-like)
    GLfloat trunk_ambient[] = {0.2f, 0.1f, 0.05f, 1.0f};
    GLfloat trunk_diffuse[] = {0.5f, 0.3f, 0.1f, 1.0f};
    GLfloat trunk_specular[] = {0.1f, 0.05f, 0.02f, 1.0f};
    GLfloat trunk_shininess = 8.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, trunk_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, trunk_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, trunk_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, trunk_shininess);

    // Draw main trunk with slight taper
    glColor3f(0.5f, 0.3f, 0.1f); // Rich brown
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric *quad = gluNewQuadric();
    gluCylinder(quad, trunkRadius, trunkRadius * 0.8f, trunkHeight * 0.7f, 12, 8);
    gluDeleteQuadric(quad);
    glPopMatrix();

    // Draw upper trunk section (thinner)
    glPushMatrix();
    glTranslatef(x, y + trunkHeight * 0.7f, z);
    glRotatef(-90, 1, 0, 0);
    quad = gluNewQuadric();
    gluCylinder(quad, trunkRadius * 0.8f, trunkRadius * 0.6f, trunkHeight * 0.3f, 12, 6);
    gluDeleteQuadric(quad);
    glPopMatrix();

    // Draw some branches
    float branchHeight = y + trunkHeight * 0.75f;
    float branchLength = trunkRadius * 2.5f;
    float branchRadius = trunkRadius * 0.3f;
    
    // Branch angles for more natural look
    float branchAngles[] = {30.0f, 120.0f, 210.0f, 300.0f};
    float branchTilts[] = {15.0f, -10.0f, 20.0f, -15.0f};
    
    glColor3f(0.4f, 0.25f, 0.1f); // Slightly darker brown for branches
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(x, branchHeight + (i * trunkHeight * 0.05f), z);
        glRotatef(branchAngles[i], 0, 1, 0); // Rotate around Y axis
        glRotatef(branchTilts[i], 0, 0, 1);  // Tilt the branch
        glRotatef(-90, 1, 0, 0);
        
        quad = gluNewQuadric();
        gluCylinder(quad, branchRadius, branchRadius * 0.5f, branchLength, 8, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    // Set material properties for foliage (leafy)
    GLfloat foliage_ambient[] = {0.05f, 0.2f, 0.05f, 1.0f};
    GLfloat foliage_diffuse[] = {0.1f, 0.6f, 0.1f, 1.0f};
    GLfloat foliage_specular[] = {0.02f, 0.1f, 0.02f, 1.0f};
    GLfloat foliage_shininess = 3.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, foliage_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, foliage_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, foliage_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, foliage_shininess);

    // Draw layered foliage for more realistic canopy
    struct FoliageLayer {
        float heightOffset, radiusScale, colorVariation;
    };
    
    FoliageLayer layers[] = {
        // Bottom layer (largest)
        {trunkHeight * 0.6f, 1.2f, 0.8f},
        // Middle layer
        {trunkHeight * 0.8f, 1.0f, 0.9f},
        // Top layer (smallest)
        {trunkHeight *  1.0f, 0.7f, 1.0f},
        // Peak
        {trunkHeight * 1.15f, 0.4f, 1.1f}
    };
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(x, y + layers[i].heightOffset, z);
        
        // Vary green color for each layer
        float greenVar = layers[i].colorVariation;
        glColor3f(0.1f * greenVar, 0.6f * greenVar, 0.1f * greenVar);
        
        glutSolidSphere(foliageRadius * layers[i].radiusScale, 14, 14);
        glPopMatrix();
    }

    // Add some smaller foliage clusters on branches for detail
    glColor3f(0.15f, 0.5f, 0.15f); // Slightly different green
    for (int i = 0; i < 4; i++) {
        float branchEndX = x + cos(branchAngles[i] * M_PI / 180.0f) * branchLength * 0.7f;
        float branchEndZ = z + sin(branchAngles[i] * M_PI / 180.0f) * branchLength * 0.7f;
        float branchEndY = branchHeight + (i * trunkHeight * 0.05f) + branchLength * sin(branchTilts[i] * M_PI / 180.0f) * 0.5f;
        
        glPushMatrix();
        glTranslatef(branchEndX, branchEndY, branchEndZ);
        glutSolidSphere(foliageRadius * 0.3f, 10, 10);
        glPopMatrix();
    }
}


void drawBush(float centerX, float centerY, float centerZ, float radius) {
    // Set material properties for bush (green, plant-like)
    GLfloat bush_ambient[] = {0.1f, 0.3f, 0.1f, 1.0f};   // Dark green ambient
    GLfloat bush_diffuse[] = {0.2f, 0.6f, 0.2f, 1.0f};   // Medium green diffuse
    GLfloat bush_specular[] = {0.05f, 0.1f, 0.05f, 1.0f}; // Very low specular (plants aren't shiny)
    GLfloat bush_shininess = 5.0f;                         // Low shininess
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, bush_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bush_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bush_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, bush_shininess);
    glColor3f(0.2f, 0.6f, 0.2f); // Green color for bush
    
    // Create a cluster of spheres to form a realistic bush
    // Define sphere offsets relative to center for bush cluster
    struct SphereOffset {
        float x, y, z, scale;
    };    // Main center sphere and surrounding spheres in cluster formation (lowered positioning)
    SphereOffset spheres[] = {
        // Main center sphere (largest)
        {0.0f, 0.0f, 0.0f, 1.3f},
        
        // Middle layer spheres (lowered more toward ground level)
        {1.0f, -0.2f, -0.7f, 0.7f},
        {-0.9f, -0.25f, 0.8f, 0.65f},
        {0.5f, -0.25f, 1.1f, 0.6f},
        
        // Lower layer spheres (majority here, closer to ground)
        {0.9f, -0.3f, 0.3f, 0.5f},
        {-1.0f, -0.4f, -0.2f, 0.55f},
        {0.3f, -0.5f, -1.0f, 0.45f}
    };
    
    int numSpheres = sizeof(spheres) / sizeof(SphereOffset);
    
    // Draw each sphere in the cluster
    for (int i = 0; i < numSpheres; i++) {
        glPushMatrix();
            // Position relative to bush center
            glTranslatef(centerX + spheres[i].x * radius, 
                        centerY + spheres[i].y * radius, 
                        centerZ + spheres[i].z * radius);
            
            // Vary the green color slightly for each sphere for more realism
            float colorVariation = 0.8f + (i % 3) * 0.1f; // Slight variation
            glColor3f(0.2f * colorVariation, 0.6f * colorVariation, 0.2f * colorVariation);
            
            // Draw sphere with scaled radius
            glutSolidSphere(radius * spheres[i].scale, 12, 12);
        glPopMatrix();
    }
}


void drawRock(float centerX, float centerY, float centerZ, float scale) {
    // Set material properties for rock (stone-like)
    GLfloat rock_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};   // Dark grey ambient
    GLfloat rock_diffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};   // Medium grey diffuse
    GLfloat rock_specular[] = {0.1f, 0.1f, 0.1f, 1.0f};  // Very low specular (rocks aren't shiny)
    GLfloat rock_shininess = 2.0f;                        // Very low shininess
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rock_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rock_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rock_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rock_shininess);

    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(scale, scale, scale);
    
    // Define vertices for an irregular rock shape
    // Using multiple interconnected triangular faces to create organic shape
    float vertices[][3] = {
        // Bottom vertices (touching ground)
        {-1.2f, 0.0f, -0.8f},    // 0
        {0.0f, 0.0f, -1.3f},     // 1  
        {1.1f, 0.0f, -0.6f},     // 2
        {1.3f, 0.0f, 0.7f},      // 3
        {0.2f, 0.0f, 1.4f},      // 4
        {-0.9f, 0.0f, 1.0f},     // 5
        {-1.5f, 0.0f, 0.2f},     // 6
        
        // Middle layer vertices
        {-0.8f, 0.6f, -0.9f},    // 7
        {0.3f, 0.7f, -1.1f},     // 8
        {1.0f, 0.5f, -0.3f},     // 9
        {1.1f, 0.8f, 0.8f},      // 10
        {-0.1f, 0.6f, 1.2f},     // 11
        {-1.0f, 0.7f, 0.5f},     // 12
        {-1.2f, 0.5f, -0.1f},    // 13
        
        // Upper layer vertices
        {-0.3f, 1.1f, -0.5f},    // 14
        {0.4f, 1.2f, -0.2f},     // 15
        {0.6f, 1.0f, 0.4f},      // 16
        {-0.2f, 1.3f, 0.6f},     // 17
        {-0.6f, 1.1f, 0.1f},     // 18
        
        // Peak vertices
        {0.0f, 1.6f, 0.1f},      // 19
        {-0.1f, 1.7f, -0.1f},    // 20
    };
    
    // Define faces using vertex indices (counter-clockwise for outward normals)
    int faces[][3] = {
        // Bottom ring faces
        {0, 1, 7}, {1, 8, 7}, {1, 2, 8}, {2, 9, 8},
        {2, 3, 9}, {3, 10, 9}, {3, 4, 10}, {4, 11, 10},
        {4, 5, 11}, {5, 12, 11}, {5, 6, 12}, {6, 13, 12},
        {6, 0, 13}, {0, 7, 13},
        
        // Middle ring faces
        {7, 8, 14}, {8, 15, 14}, {8, 9, 15}, {9, 16, 15},
        {9, 10, 16}, {10, 17, 16}, {10, 11, 17}, {11, 18, 17},
        {11, 12, 18}, {12, 19, 18}, {12, 13, 19}, {13, 14, 19},
        {13, 7, 14},
        
        // Upper ring faces
        {14, 15, 20}, {15, 16, 20}, {16, 17, 20}, {17, 18, 20},
        {18, 19, 20}, {19, 14, 20},
        
        // Additional irregular faces for more organic shape
        {7, 12, 18}, {7, 18, 14}, {8, 9, 16}, {8, 16, 15},
        {10, 11, 17}, {12, 13, 19}, {14, 18, 19},
        
        // Some inverted faces for surface irregularities
        {1, 0, 6}, {1, 6, 4}, {2, 1, 4}, {2, 4, 3},
        {15, 16, 19}, {15, 19, 18}, {15, 18, 14},
    };
    
    int numFaces = sizeof(faces) / sizeof(faces[0]);
    
    // Draw the rock using triangular faces
    glBegin(GL_TRIANGLES);
    
    for (int i = 0; i < numFaces; i++) {
        // Get the three vertices of the triangle
        float* v1 = vertices[faces[i][0]];
        float* v2 = vertices[faces[i][1]];
        float* v3 = vertices[faces[i][2]];
        
        // Calculate normal vector (cross product)
        float edge1[3] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
        float edge2[3] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
        
        float normal[3] = {
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0]
        };
        
        // Normalize the normal vector
        float length = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        if (length > 0.001f) {
            normal[0] /= length;
            normal[1] /= length;
            normal[2] /= length;
        }
        
        // Add slight color variation for each face to simulate rock texture
        float colorVar = 0.8f + (i % 5) * 0.05f;
        glColor3f(0.5f * colorVar, 0.5f * colorVar, 0.5f * colorVar);
        
        // Draw the triangle with calculated normal
        glNormal3f(normal[0], normal[1], normal[2]);
        glVertex3f(v1[0], v1[1], v1[2]);
        glVertex3f(v2[0], v2[1], v2[2]);
        glVertex3f(v3[0], v3[1], v3[2]);
    }
    
    glEnd();
    
    // Add some surface detail bumps for more realistic texture
    glColor3f(0.4f, 0.4f, 0.4f); // Darker grey for details
    
    // Small surface bumps
    struct BumpDetail {
        float x, y, z, size;
    };
    
    BumpDetail bumps[] = {
        {-0.3f, 0.8f, -0.2f, 0.1f},
        {0.4f, 0.6f, 0.3f, 0.08f},
        {-0.6f, 0.4f, 0.5f, 0.12f},
        {0.7f, 0.9f, -0.1f, 0.07f},
        {-0.1f, 1.1f, 0.4f, 0.09f},
        {0.2f, 0.3f, -0.7f, 0.11f}
    };
    
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        glTranslatef(bumps[i].x, bumps[i].y, bumps[i].z);
        
        // Create small irregular bump using a few triangles
        glBegin(GL_TRIANGLES);
        float size = bumps[i].size;
        
        // Simple pyramid-like bump
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-size, 0.0f, -size);
        glVertex3f(size, 0.0f, -size);
        glVertex3f(0.0f, size * 0.8f, 0.0f);
        
        glVertex3f(size, 0.0f, -size);
        glVertex3f(size, 0.0f, size);
        glVertex3f(0.0f, size * 0.8f, 0.0f);
        
        glVertex3f(size, 0.0f, size);
        glVertex3f(-size, 0.0f, size);
        glVertex3f(0.0f, size * 0.8f, 0.0f);
        
        glVertex3f(-size, 0.0f, size);
        glVertex3f(-size, 0.0f, -size);
        glVertex3f(0.0f, size * 0.8f, 0.0f);
        
        glEnd();
        glPopMatrix();
    }
    
    glPopMatrix();
}
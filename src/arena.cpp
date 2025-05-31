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
std::vector<ArenaCube> cubes;
std::vector<ArenaRamp> ramps;

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

// --- Arena Building Helper Functions ---
// Fungsi addFlatArea dan addRampArea tidak lagi memodifikasi arenaHeights secara langsung
// Kubus dan Ramp akan digambar secara eksplisit

void drawCube(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ) {
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(sizeX, sizeY, sizeZ);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawRamp(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ, char axis) {
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ); // Pindahkan ke pusat ramp

    // Setengah ukuran untuk kemudahan perhitungan vertex
    // sizeY adalah tinggi total ramp. Puncak ramp akan di centerY + sizeY/2, dasar di centerY - sizeY/2
    float hx = sizeX / 2.0f;
    float hy = sizeY / 2.0f; 
    float hz = sizeZ / 2.0f;

    if (axis == 'z') { // Ramp miring sepanjang sumbu Z
        // Vertices (koordinat lokal relatif terhadap pusat ramp yang sudah ditranslasikan)
        // Dasar ramp berada di y = -hy, puncak ramp di y = hy.
        // Ramp miring dari z = -hz (kaki ramp) ke z = hz (puncak ramp, bagian vertikal).
        float v[6][3] = {
            {-hx, -hy, -hz}, // v0 (depan-bawah-kiri, kaki ramp)
            { hx, -hy, -hz}, // v1 (depan-bawah-kanan, kaki ramp)
            {-hx, -hy,  hz}, // v2 (belakang-bawah-kiri, di dasar bagian vertikal)
            { hx, -hy,  hz}, // v3 (belakang-bawah-kanan, di dasar bagian vertikal)
            {-hx,  hy,  hz}, // v4 (belakang-atas-kiri, puncak ramp)
            { hx,  hy,  hz}  // v5 (belakang-atas-kanan, puncak ramp)
        };

        // Normal untuk permukaan miring (v0, v1, v5, v4)
        // Vektor dari v0 ke v1: (2*hx, 0, 0)
        // Vektor dari v0 ke v4: (0, 2*hy, 2*hz) -> (0, sizeY, sizeZ)
        // Cross product: (0 * sizeZ - 0 * sizeY, 0 * 0 - 2*hx * sizeZ, 2*hx * sizeY - 0*0)
        // = (0, -sizeX * sizeZ, sizeX * sizeY)
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
        // Pastikan normal mengarah "keluar" dari permukaan atas.
        // Jika sizeY positif, kita ingin komponen Y dari normal (setelah transformasi) positif.
        // Untuk ramp Z, normal permukaan miring adalah (0, cos(angle), sin(angle)) jika angle adalah sudut kemiringan dari XZ
        // atau lebih tepatnya, normalnya adalah (0, sizeZ, -sizeY) dinormalisasi, jika ramp naik ke +Y saat Z meningkat.
        // Jika ramp naik dari -hy ke +hy, dan dari -hz ke +hz (bagian atas di +hz)
        // Vektor pada permukaan: (sizeX, 0, 0) dan (0, sizeY, sizeZ)
        // Normal: (0, -sizeX*sizeZ, sizeX*sizeY)
        // Kita ingin normal yang Y-nya positif jika dilihat dari atas.
        // Vektor kemiringan: (0, sizeY, sizeZ)
        // Vektor lebar: (sizeX, 0, 0)
        // Normal permukaan miring: cross((sizeX,0,0), (0,sizeY,sizeZ)) = (0, -sizeX*sizeZ, sizeX*sizeY)
        // Ini adalah normal yang benar jika sisi depan (v0,v1) lebih rendah dari sisi belakang (v4,v5)
        // Untuk memastikan normal mengarah ke atas relatif terhadap kemiringan:
        // N = (0, sizeZ, -sizeY) (jika ramp naik dari -Z ke +Z)
        // Jika ramp naik dari Z=-hz ke Z=+hz, maka kaki di -hz, puncak di +hz.
        // Permukaan miring: v0,v1,v5,v4
        // v0=(-hx,-hy,-hz), v1=(hx,-hy,-hz), v4=(-hx,hy,hz), v5=(hx,hy,hz)
        // E1 = v1-v0 = (2hx, 0, 0)
        // E2 = v4-v0 = (0, 2hy, 2hz)
        // N = E1 x E2 = (0, -2hx*2hz, 2hx*2hy) = (0, -sizeX*sizeZ, sizeX*sizeY)
        // Normal ini memiliki komponen Y negatif jika sizeX, sizeY, sizeZ positif. Kita balik.
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
        // Vertices (koordinat lokal relatif terhadap pusat ramp)
        // Dasar ramp di y = -hy, puncak ramp di y = hy.
        // Ramp miring dari x = -hx (kaki ramp) ke x = hx (puncak ramp, bagian vertikal).
        float v[6][3] = {
            {-hx, -hy, -hz}, // v0 (kiri-bawah-depan, kaki ramp)
            {-hx, -hy,  hz}, // v1 (kiri-bawah-belakang, kaki ramp)
            { hx, -hy, -hz}, // v2 (kanan-bawah-depan, di dasar bagian vertikal)
            { hx, -hy,  hz}, // v3 (kanan-bawah-belakang, di dasar bagian vertikal)
            { hx,  hy, -hz}, // v4 (kanan-atas-depan, puncak ramp)
            { hx,  hy,  hz}  // v5 (kanan-atas-belakang, puncak ramp)
        };

        // Normal untuk permukaan miring (v0, v1, v5, v4)
        // E1 = v1-v0 = (0, 0, 2hz)
        // E2 = v4-v0 = (2hx, 2hy, 0)
        // N = E1 x E2 = (-2hz*2hy, 2hz*2hx, 0) = (-sizeZ*sizeY, sizeZ*sizeX, 0)
        // Kita balik agar Y positif.
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
    }

    glPopMatrix();
}


void setupArenaGeometry() {
    cubes.clear(); ramps.clear();
    // Contoh penggunaan:
    // CreateCube(0.0f, 2.0f, 5.0f, 2.0f, 2.0f, 2.0f); // Cube di tengah
    // CreateRamp(0.0f, 1.0f, 2.5f, 2.0f, 2.0f, 3.0f, 'z'); // Ramp ke cube
    CreateRamp(1.0f, 1.0f, -2.5f, 2.0f, 2.0f, 3.0f, 'x'); // Ramp ke arah X
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
    CreateRamp(28.67f, 3.5f, 31.09f, 1.0f, 3.0f, 6.0f, 'z');
    CreateCube(28.67f, 4.5f, 38.09f, 8.0f, 1.0f, 8.0f);


    // Developer bisa tambah sendiri: CreateCube(...), CreateRamp(...)
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


// --- Debug Helper: Print Marble Position for Object Placement ---
// Call this function from your input handler (e.g., when 'O' is pressed)
// Replace 'marble.x', 'marble.y', 'marble.z' with your actual marble variable names if different
void PrintMarblePositionForPlacement(float x, float y, float z) {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("CreateCube(%.2ff, %.2ff, %.2ff, sizeX, sizeY, sizeZ);\n", x, y, z);
    printf("CreateRamp(%.2ff, %.2ff, %.2ff, sizeX, sizeY, sizeZ, 'axis');\n", x, y, z);
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
}
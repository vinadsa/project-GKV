#include "arena.h"
#include "globals.h" 
#include "utils.h"   
#include <cmath>     
#include <GL/glut.h>
#include <cstdio>    
#include <vector>    




static const float testCubeX = 0.0f;
static const float testCubeY = 2.0f;     
static const float testCubeZ = 5.0f;
static const float testCubeSizeX = 2.0f;
static const float testCubeSizeY = 2.0f;  
static const float testCubeSizeZ = 2.0f;

static const float testRampX = 0.0f;
static const float testRampY = 1.0f;     
static const float testRampZ = 2.5f;     
static const float testRampSizeX = 2.0f; 
static const float testRampSizeY = 2.0f;
static const float testRampSizeZ = 3.0f; 
static const char testRampAxis = 'z';

// Definisi array global arenaHeights 
float arenaHeights[GRID_SIZE][GRID_SIZE] = {0};

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

#include <vector>
struct Coin {
    float x, y, z;
    bool collected;
};
std::vector<Coin> coins;

void addCoin(float x, float z) {
    float groundH, nx, ny, nz;
    getArenaHeightAndNormal(x, z, groundH, nx, ny, nz);
    coins.push_back({x, groundH + 0.5f, z, false});
}

float coinSpinAngle = 0.0f;
float coinBounceTime = 0.0f;

void updateCoinAnimation(float deltaTime) {
    coinSpinAngle += 120.0f * deltaTime; // 120 derajat per detik
    if (coinSpinAngle > 360.0f) coinSpinAngle -= 360.0f;
    coinBounceTime += deltaTime;
}

void drawCoins() {
    for (const Coin& coin : coins) {
        if (coin.collected) continue;
        glPushMatrix();
        // Animasi naik turun
        float bounce = 0.2f * sinf(coinBounceTime * 2.5f + coin.x + coin.z);
        glTranslatef(coin.x, coin.y + bounce, coin.z);
        // Animasi rotasi
        glRotatef(coinSpinAngle, 0, 1, 0);
        // Material: Emas (gold)
        GLfloat gold_ambient[4] = {0.24725f, 0.1995f, 0.0745f, 1.0f};
        GLfloat gold_diffuse[4] = {0.75164f, 0.60648f, 0.22648f, 1.0f};
        GLfloat gold_specular[4] = {0.628281f, 0.555802f, 0.366065f, 1.0f};
        GLfloat gold_shininess = 51.2f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gold_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gold_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gold_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, gold_shininess);
        glColor3f(1.0f, 0.84f, 0.0f); // Gold color
        GLUquadric* quad = gluNewQuadric();
        if (quad) {
            gluDisk(quad, 0.0, 0.4, 32, 1); // Sisi atas
            glTranslatef(0, 0.08f, 0);
            gluDisk(quad, 0.0, 0.4, 32, 1); // Sisi bawah
            glTranslatef(0, -0.04f, 0);
            gluCylinder(quad, 0.4, 0.4, 0.08, 32, 1); // Sisi samping
            gluDeleteQuadric(quad);
        }
        glPopMatrix();
    }
}

void CreateCube(float x, float y, float z, float sizeX, float sizeY, float sizeZ) {
    cubes.push_back({x, y, z, sizeX, sizeY, sizeZ});
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

void drawCube(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ) {
    if (enableShadows) {
        glPushMatrix();
            GLfloat shadow_plane[4] = {0.0f, 1.0f, 0.0f, -0.01f}; 
            GLfloat shadow_light[4] = {10.0f, 80.0f, 10.0f, 1.0f};
            extern void glShadowProjection(const float*, const float*); 
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

    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(sizeX, sizeY, sizeZ);
    glColor3f(0.7f, 0.6f, 0.5f); 
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawRamp(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ, char axis) {
    if (enableShadows) {
        glPushMatrix();
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
            
            if (axis == 'z') {
                float v[6][3] = {
                    {-hx, -hy, -hz}, { hx, -hy, -hz}, {-hx, -hy,  hz}, 
                    { hx, -hy,  hz}, {-hx,  hy,  hz}, { hx,  hy,  hz}
                };
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
                glEnd();
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[2]); glVertex3fv(v[3]); glVertex3fv(v[1]);
                glEnd();
            } else if (axis == 'x') {
                float v[6][3] = {
                    {-hx, -hy, -hz}, {-hx, -hy,  hz}, { hx, -hy, -hz}, 
                    { hx, -hy,  hz}, { hx,  hy, -hz}, { hx,  hy,  hz}
                };
                glBegin(GL_QUADS);
                glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
                glEnd();
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

    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ); // Pindahkan ke pusat ramp
    
    glColor3f(0.7f, 0.6f, 0.5f); 

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
        float norm_slope_y = -sizeX * sizeZ; 
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
        len_slope = sqrt(norm_slope_y*norm_slope_y + norm_slope_z*norm_slope_z); 
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
        float len_slope = sqrt(norm_slope_x*norm_slope_x + norm_slope_y*norm_slope_y); 
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
    CreateRamp(1.0f, 1.0f, -3.0f, 2.0f, 2.0f, 1.0f, 'x'); 
    CreateCube(7.0f, 2.0f, -3.0f, 10.0f, 1.0f, 1.0f); 
    CreateCube(10.0f, 2.0f, -3.0f, 1.0f, 1.0f, 10.0f); 
    CreateRamp(10.0f, 2.7f, 1.25f, 2.0f, 2.0f, 7.0f, 'z');
    CreateCube(10.17f, 0.50f, 6.09f, 1.0f, 1.0f, 1.0f);          
    CreateRamp(10.17f, 0.50f, 6.09f, 1.0f, 1.0f, 1.0f, 'z');    
 
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

    CreateBush(-5.0f, 0.5f, -10.0f, 0.8f);   
    CreateBush(-3.0f, 0.4f, -8.0f, 0.6f);   
    CreateBush(5.0f, 0.7f, 0.0f, 0.7f);    
    CreateBush(15.0f, 0.6f, 15.0f, 0.7f);  
    CreateBush(25.0f, 0.5f, 10.0f, 0.7f);   
    CreateBush(-10.50f, 0.50f, 13.14f, 0.5f);
    CreateBush(30.0f, 0.4f, 30.0f, 0.5f);   
    CreateBush(-1.44f, 0.50f, 28.83f, 0.7f);
    CreateBush(20.71f, 0.50f, -12.45f, 0.7f);

    
    CreateTree(21.71f, 0.30f, -10.45f, 10.0f, 0.2f, 1.5f); 
    CreateTree(6.71f, 0.30f, -10.45f, 7.0f, 0.2f, 1.5f); 
    CreateTree(15.71f, 0.30f, 9.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-5.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-7.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-10.71f, 0.30f, 27.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-11.71f, 0.30f, -14.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-13.71f, 0.30f, -14.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(14.71f, 0.30f, -21.45f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-33.28f, 0.50f, -9.32f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-33.42f, 0.50f, 23.98f, 5.0f, 0.2f, 1.5f); 
    CreateTree(-33.88f, 0.50f, 31.04f, 5.0f, 0.2f, 1.5f); 


    CreateRock(18.0f, 0.0f, 12.0f, 1.2f);   
    CreateRock(-8.0f, 0.0f, -20.0f, 0.8f);   
    CreateRock(25.0f, 0.0f, -15.0f, 1.5f);   
    CreateRock(-12.0f, 0.0f, 15.0f, 0.6f);  
    CreateRock(10.0f, 0.0f, 25.0f, 1.0f);   
    CreateRock(-25.0f, 0.0f, 5.0f, 1.3f);   
    CreateRock(5.0f, 0.0f, -25.0f, 0.9f);    
    CreateRock(-29.91f, 0.50f, -16.21f, 1.3f);   
    CreateRock(-25.89f, 0.50f, 29.16f, 0.9f);    
}


float getArenaHeight(float x, float z) {
    float height = 0.0f; 
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
    height = 0.0f;
    outNormalX = 0.0f; outNormalY = 1.0f; outNormalZ = 0.0f;
    const float epsilon = 0.015f;
    const float epsilon_normal_Y_diff = 0.05f;
    const float wall_like_threshold_Y = 0.5f;
    float current_best_h = height;
    float current_best_nx = outNormalX, current_best_ny = outNormalY, current_best_nz = outNormalZ;
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

float getArenaHeightAt(float x, float y, float z) {
    float height = 0.0f; 
    const float epsilon = 0.01f;
    for (const auto& c : cubes) {
        float minX = c.x - c.sizeX / 2.0f;
        float maxX = c.x + c.sizeX / 2.0f;
        float minZ = c.z - c.sizeZ / 2.0f;
        float maxZ = c.z + c.sizeZ / 2.0f;
        float topY = c.y + c.sizeY / 2.0f;
        float bottomY = c.y - c.sizeY / 2.0f;
        if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            if (y >= topY - epsilon) {
                if (topY > height) height = topY;
            }
        }
    }
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
    height = 0.0f;
    outNormalX = 0.0f; outNormalY = 1.0f; outNormalZ = 0.0f;
    const float epsilon = 0.015f;
    const float epsilon_normal_Y_diff = 0.05f;
    const float wall_like_threshold_Y = 0.5f;
        float current_best_h = height;
    float current_best_nx = outNormalX, current_best_ny = outNormalY, current_best_nz = outNormalZ;
    
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
                if (y >= obj_h - epsilon) {
                    float len = sqrt(obj_nx*obj_nx + obj_ny*obj_ny + obj_nz*obj_nz);
                    if (len > 1e-6) { obj_nx /= len; obj_ny /= len; obj_nz /= len; }
                    if (obj_ny < 0) { obj_nx *= -1; obj_ny *= -1; obj_nz *= -1; }
                    obj_is_wall = (obj_ny < wall_like_threshold_Y);
                } else {
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

void drawGround() {
    GLfloat ground_ambient[] = {0.1f, 0.4f, 0.1f, 1.0f};
    GLfloat ground_diffuse[] = {0.2f, 0.8f, 0.2f, 1.0f};
    GLfloat ground_specular[] = {0.1f, 0.2f, 0.1f, 1.0f};
    GLfloat ground_shininess = 8.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ground_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ground_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ground_shininess);
    glColor3f(0.2f, 0.8f, 0.2f); 

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

    for (const auto& b : bushes) {
        drawBush(b.x, b.y, b.z, b.radius);
    }
    for (const auto& t : trees) {
        drawTree(t.x, t.y, t.z, t.trunkHeight, t.trunkRadius, t.foliageRadius);
    }
    
    for (const auto& rock : rocks) {
        drawRock(rock.x, rock.y, rock.z, rock.scale);
    }

    drawCoins();
}


void drawTree(float x, float y, float z, float trunkHeight, float trunkRadius, float foliageRadius) {
    GLfloat trunk_ambient[] = {0.2f, 0.1f, 0.05f, 1.0f};
    GLfloat trunk_diffuse[] = {0.5f, 0.3f, 0.1f, 1.0f};
    GLfloat trunk_specular[] = {0.1f, 0.05f, 0.02f, 1.0f};
    GLfloat trunk_shininess = 8.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, trunk_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, trunk_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, trunk_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, trunk_shininess);

    glColor3f(0.5f, 0.3f, 0.1f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric *quad = gluNewQuadric();
    gluCylinder(quad, trunkRadius, trunkRadius * 0.8f, trunkHeight * 0.7f, 12, 8);
    gluDeleteQuadric(quad);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + trunkHeight * 0.7f, z);
    glRotatef(-90, 1, 0, 0);
    quad = gluNewQuadric();
    gluCylinder(quad, trunkRadius * 0.8f, trunkRadius * 0.6f, trunkHeight * 0.3f, 12, 6);
    gluDeleteQuadric(quad);
    glPopMatrix();

    float branchHeight = y + trunkHeight * 0.75f;
    float branchLength = trunkRadius * 2.5f;
    float branchRadius = trunkRadius * 0.3f;
    
    float branchAngles[] = {30.0f, 120.0f, 210.0f, 300.0f};
    float branchTilts[] = {15.0f, -10.0f, 20.0f, -15.0f};
    
    glColor3f(0.4f, 0.25f, 0.1f); 
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(x, branchHeight + (i * trunkHeight * 0.05f), z);
        glRotatef(branchAngles[i], 0, 1, 0);
        glRotatef(branchTilts[i], 0, 0, 1); 
        glRotatef(-90, 1, 0, 0);
        
        quad = gluNewQuadric();
        gluCylinder(quad, branchRadius, branchRadius * 0.5f, branchLength, 8, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    GLfloat foliage_ambient[] = {0.05f, 0.2f, 0.05f, 1.0f};
    GLfloat foliage_diffuse[] = {0.1f, 0.6f, 0.1f, 1.0f};
    GLfloat foliage_specular[] = {0.02f, 0.1f, 0.02f, 1.0f};
    GLfloat foliage_shininess = 3.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, foliage_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, foliage_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, foliage_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, foliage_shininess);

    struct FoliageLayer {
        float heightOffset, radiusScale, colorVariation;
    };
    
    FoliageLayer layers[] = {
        {trunkHeight * 0.6f, 1.2f, 0.8f},
        {trunkHeight * 0.8f, 1.0f, 0.9f},
        {trunkHeight *  1.0f, 0.7f, 1.0f},
        {trunkHeight * 1.15f, 0.4f, 1.1f}
    };
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(x, y + layers[i].heightOffset, z);
        
        float greenVar = layers[i].colorVariation;
        glColor3f(0.1f * greenVar, 0.6f * greenVar, 0.1f * greenVar);
        
        glutSolidSphere(foliageRadius * layers[i].radiusScale, 14, 14);
        glPopMatrix();
    }

    glColor3f(0.15f, 0.5f, 0.15f); 
    for (int i = 0; i < 4; i++) {
        float branchEndX = x + cos(branchAngles[i] * M_PI /  180.0f) * branchLength * 0.7f;
        float branchEndZ = z + sin(branchAngles[i] * M_PI / 180.0f) * branchLength * 0.7f;
        float branchEndY = branchHeight + (i * trunkHeight * 0.05f) + branchLength * sin(branchTilts[i] * M_PI / 180.0f) * 0.5f;
        
        glPushMatrix();
        glTranslatef(branchEndX, branchEndY, branchEndZ);
        glutSolidSphere(foliageRadius * 0.3f, 10, 10);
        glPopMatrix();
    }
}


void drawBush(float centerX, float centerY, float centerZ, float radius) {
    GLfloat bush_ambient[] = {0.1f, 0.3f, 0.1f, 1.0f};  
    GLfloat bush_diffuse[] = {0.2f, 0.6f, 0.2f, 1.0f};  
    GLfloat bush_specular[] = {0.05f, 0.1f, 0.05f, 1.0f};
    GLfloat bush_shininess = 5.0f;                   
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, bush_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bush_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bush_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, bush_shininess);
    glColor3f(0.2f, 0.6f, 0.2f); 
    
    struct SphereOffset {
        float x, y, z, scale;
    };    
    SphereOffset spheres[] = {
        {0.0f, 0.0f, 0.0f, 1.3f},
        
        {1.0f, -0.2f, -0.7f, 0.7f},
        {-0.9f, -0.25f, 0.8f, 0.65f},
        {0.5f, -0.25f, 1.1f, 0.6f},
        
        {0.9f, -0.3f, 0.3f, 0.5f},
        {-1.0f, -0.4f, -0.2f, 0.55f},
        {0.3f, -0.5f, -1.0f, 0.45f}
    };
    
    int numSpheres = sizeof(spheres) / sizeof(SphereOffset);
    
    for (int i = 0; i < numSpheres; i++) {
        glPushMatrix();
            glTranslatef(centerX + spheres[i].x * radius, 
                        centerY + spheres[i].y * radius, 
                        centerZ + spheres[i].z * radius);
            
            float colorVariation = 0.8f + (i % 3) * 0.1f; // Slight variation
            glColor3f(0.2f * colorVariation, 0.6f * colorVariation, 0.2f * colorVariation);
            glutSolidSphere(radius * spheres[i].scale, 12, 12);
        glPopMatrix();
    }
}


void drawRock(float centerX, float centerY, float centerZ, float scale) {
    GLfloat rock_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};  
    GLfloat rock_diffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};  
    GLfloat rock_specular[] = {0.1f, 0.1f, 0.1f, 1.0f};  
    GLfloat rock_shininess = 2.0f;                        
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rock_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rock_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rock_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rock_shininess);

    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(scale, scale, scale);
    
    float vertices[][3] = {
        {-1.2f, 0.0f, -0.8f},    // 0
        {0.0f, 0.0f, -1.3f},     // 1  
        {1.1f, 0.0f, -0.6f},     // 2
        {1.3f, 0.0f, 0.7f},      // 3
        {0.2f, 0.0f, 1.4f},      // 4
        {-0.9f, 0.0f, 1.0f},     // 5
        {-1.5f, 0.0f, 0.2f},     // 6
        
        {-0.8f, 0.6f, -0.9f},    // 7
        {0.3f, 0.7f, -1.1f},     // 8
        {1.0f, 0.5f, -0.3f},     // 9
        {1.1f, 0.8f, 0.8f},      // 10
        {-0.1f, 0.6f, 1.2f},     // 11
        {-1.0f, 0.7f, 0.5f},     // 12
        {-1.2f, 0.5f, -0.1f},    // 13

        {-0.3f, 1.1f, -0.5f},    // 14
        {0.4f, 1.2f, -0.2f},     // 15
        {0.6f, 1.0f, 0.4f},      // 16
        {-0.2f, 1.3f, 0.6f},     // 17
        {-0.6f, 1.1f, 0.1f},     // 18
        
        {0.0f, 1.6f, 0.1f},      // 19
        {-0.1f, 1.7f, -0.1f},    // 20
    };
    
    int faces[][3] = {
        {0, 1, 7}, {1, 8, 7}, {1, 2, 8}, {2, 9, 8},
        {2, 3, 9}, {3, 10, 9}, {3, 4, 10}, {4, 11, 10},
        {4, 5, 11}, {5, 12, 11}, {5, 6, 12}, {6, 13, 12},
        {6, 0, 13}, {0, 7, 13},
        
        {7, 8, 14}, {8, 15, 14}, {8, 9, 15}, {9, 16, 15},
        {9, 10, 16}, {10, 17, 16}, {10, 11, 17}, {11, 18, 17},
        {11, 12, 18}, {12, 19, 18}, {12, 13, 19}, {13, 14, 19},
        {13, 7, 14},
        
        {14, 15, 20}, {15, 16, 20}, {16, 17, 20}, {17, 18, 20},
        {18, 19, 20}, {19, 14, 20},
        
        {7, 12, 18}, {7, 18, 14}, {8, 9, 16}, {8, 16, 15},
        {10, 11, 17}, {12, 13, 19}, {14, 18, 19},
        
        {1, 0, 6}, {1, 6, 4}, {2, 1, 4}, {2, 4, 3},
        {15, 16, 19}, {15, 19, 18}, {15, 18, 14},
    };
    
    int numFaces = sizeof(faces) / sizeof(faces[0]);
    
    glBegin(GL_TRIANGLES);
    
    for (int i = 0; i < numFaces; i++) {
        float* v1 = vertices[faces[i][0]];
        float* v2 = vertices[faces[i][1]];
        float* v3 = vertices[faces[i][2]];
        
        float edge1[3] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
        float edge2[3] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
        
        float normal[3] = {
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0]
        };
        
        float length = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        if (length > 0.001f) {
            normal[0] /= length;
            normal[1] /= length;
            normal[2] /= length;
        }
        
        float colorVar = 0.8f + (i % 5) * 0.05f;
        glColor3f(0.5f * colorVar, 0.5f * colorVar, 0.5f * colorVar);
        
        glNormal3f(normal[0], normal[1], normal[2]);
        glVertex3f(v1[0], v1[1], v1[2]);
        glVertex3f(v2[0], v2[1], v2[2]);
        glVertex3f(v3[0], v3[1], v3[2]);
    }
    
    glEnd();
    
    glColor3f(0.4f, 0.4f, 0.4f); 
    
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
        
        glBegin(GL_TRIANGLES);
        float size = bumps[i].size;
        
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
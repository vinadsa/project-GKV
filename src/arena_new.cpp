#include "arena.h"
#include "globals.h"
#include "utils.h"
#include <cmath>
#include <GL/glut.h>

// Global containers untuk objek 3D
std::vector<Cube3D> worldCubes;
std::vector<Triangle3D> worldTriangles;
std::vector<Platform> worldPlatforms;

// Arena bounds
float arenaMinX = -BOUNDS;
float arenaMaxX = BOUNDS;
float arenaMinZ = -BOUNDS;
float arenaMaxZ = BOUNDS;
float arenaFloorY = 0.0f;  // Floor level di koordinat 0

// --- 3D Object Creation Functions ---
void addCube3D(float x, float y, float z, float width, float height, float depth, float r, float g, float b) {
    Cube3D cube = {x, y, z, width, height, depth, r, g, b};
    worldCubes.push_back(cube);
}

// Fungsi untuk membuat ramp segitiga siku-siku 3D
void addTriangleRamp(float startX, float startZ, float endX, float endZ, float startHeight, float endHeight, float width, float r, float g, float b) {
    // Buat beberapa segitiga untuk membentuk ramp yang halus
    float steps = 5.0f; // Jumlah segitiga untuk membuat ramp
    float stepX = (endX - startX) / steps;
    float stepZ = (endZ - startZ) / steps;
    float stepHeight = (endHeight - startHeight) / steps;
    
    for (int i = 0; i < (int)steps; i++) {
        float currentX = startX + stepX * i;
        float currentZ = startZ + stepZ * i;
        float currentHeight = startHeight + stepHeight * i;
        float nextHeight = startHeight + stepHeight * (i + 1);
        
        // Membuat segitiga siku-siku untuk setiap step
        addTriangle3D(currentX, currentHeight, currentZ - width/2,           // Bottom left
                     currentX + stepX, nextHeight, currentZ + stepZ - width/2, // Top right  
                     currentX, currentHeight, currentZ + width/2,             // Bottom right
                     r, g, b);
                     
        // Segitiga kedua untuk melengkapi persegi
        addTriangle3D(currentX + stepX, nextHeight, currentZ + stepZ - width/2, // Top right
                     currentX + stepX, nextHeight, currentZ + stepZ + width/2,   // Top left
                     currentX, currentHeight, currentZ + width/2,               // Bottom right
                     r, g, b);
    }
}

void addTriangle3D(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float r, float g, float b) {
    Triangle3D triangle = {x1, y1, z1, x2, y2, z2, x3, y3, z3, r, g, b};
    worldTriangles.push_back(triangle);
}

void addPlatform(float x, float y, float z, float width, float depth, float height, float r, float g, float b) {
    Platform platform = {x, y, z, width, depth, height, r, g, b};
    worldPlatforms.push_back(platform);
}

// --- Physics Functions ---
float getFloorHeight(float x, float z, float currentMarbleY) {
    float highestY = arenaFloorY;
    
    // Check collision dengan semua platform
    for (const Platform& platform : worldPlatforms) {
        float halfWidth = platform.width / 2.0f;
        float halfDepth = platform.depth / 2.0f;
        float platformTop = platform.y + platform.height / 2.0f;
        
        // Cek apakah marble berada di atas platform
        if (x >= platform.x - halfWidth && x <= platform.x + halfWidth &&
            z >= platform.z - halfDepth && z <= platform.z + halfDepth) {
            
            // Batasan ketinggian: marble hanya bisa naik ke platform jika:
            // 1. Platform tidak terlalu tinggi dari posisi marble saat ini
            // 2. Atau marble sudah berada di dekat platform tersebut
            float heightDifference = platformTop - currentMarbleY;
            if (heightDifference <= maxJumpHeight || 
                (currentMarbleY >= platformTop - marbleRadius - 0.5f && platformTop > highestY)) {
                highestY = platformTop;
            }
        }
    }
    
    // Check collision dengan semua kubus
    for (const Cube3D& cube : worldCubes) {
        float halfWidth = cube.width / 2.0f;
        float halfHeight = cube.height / 2.0f;
        float halfDepth = cube.depth / 2.0f;
        float cubeTop = cube.y + halfHeight;
        
        // Cek apakah marble berada di atas kubus
        if (x >= cube.x - halfWidth && x <= cube.x + halfWidth &&
            z >= cube.z - halfDepth && z <= cube.z + halfDepth) {
            
            // Batasan ketinggian: marble hanya bisa naik ke kubus jika:
            // 1. Kubus tidak terlalu tinggi dari posisi marble saat ini
            // 2. Atau marble sudah berada di dekat kubus tersebut
            float heightDifference = cubeTop - currentMarbleY;
            if (heightDifference <= maxJumpHeight || 
                (currentMarbleY >= cubeTop - marbleRadius - 0.5f && cubeTop > highestY)) {
                highestY = cubeTop;
            }
        }
    }
    
    // Check collision dengan triangles (untuk ramp)
    for (const Triangle3D& triangle : worldTriangles) {
        // Simple point-in-triangle check dan interpolasi ketinggian
        // Ini adalah implementasi sederhana untuk triangle collision
        float minX = fmin(fmin(triangle.x1, triangle.x2), triangle.x3);
        float maxX = fmax(fmax(triangle.x1, triangle.x2), triangle.x3);
        float minZ = fmin(fmin(triangle.z1, triangle.z2), triangle.z3);
        float maxZ = fmax(fmax(triangle.z1, triangle.z2), triangle.z3);
        
        if (x >= minX && x <= maxX && z >= minZ && z <= maxZ) {
            // Interpolasi sederhana ketinggian berdasarkan posisi
            float avgY = (triangle.y1 + triangle.y2 + triangle.y3) / 3.0f;
            if (avgY > highestY && (avgY - currentMarbleY) <= maxJumpHeight + 1.0f) {
                highestY = avgY;
            }
        }
    }
    
    return highestY;
}

bool checkCollisionWithObjects(float x, float y, float z, float radius) {
    // Check collision dengan kubus
    for (const Cube3D& cube : worldCubes) {
        float halfWidth = cube.width / 2.0f;
        float halfHeight = cube.height / 2.0f;
        float halfDepth = cube.depth / 2.0f;
        
        // Simple AABB collision
        if (x + radius > cube.x - halfWidth && x - radius < cube.x + halfWidth &&
            y + radius > cube.y - halfHeight && y - radius < cube.y + halfHeight &&
            z + radius > cube.z - halfDepth && z - radius < cube.z + halfDepth) {
            return true;
        }
    }
    
    return false;
}

void getArenaHeightAndNormal(float x, float z, float& outHeight, float& outNormalX, float& outNormalY, float& outNormalZ) {
    // Gunakan marbleY yang sudah ada dari globals untuk konteks
    extern float marbleY; 
    outHeight = getFloorHeight(x, z, marbleY);
    
    // Default normal pointing up
    outNormalX = 0.0f;
    outNormalY = 1.0f;
    outNormalZ = 0.0f;
    
    // Bisa diperluas untuk menghitung normal yang lebih akurat berdasarkan surface
}

float getArenaHeight(float x, float z) {
    // Gunakan marbleY yang sudah ada dari globals untuk konteks
    extern float marbleY;
    return getFloorHeight(x, z, marbleY);
}

// --- Drawing Functions ---
void draw3DArena() {
    // Draw floor (large platform) - lebih tinggi dan lebih terlihat
    glPushMatrix();
    glTranslatef(0.0f, arenaFloorY, 0.0f);
    glColor3f(0.3f, 0.3f, 0.3f); // Dark gray floor, sedikit lebih terang
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-BOUNDS, 0.0f, -BOUNDS);
    glVertex3f(BOUNDS, 0.0f, -BOUNDS);
    glVertex3f(BOUNDS, 0.0f, BOUNDS);
    glVertex3f(-BOUNDS, 0.0f, BOUNDS);
    glEnd();
    glPopMatrix();
    
    // Draw platforms
    for (const Platform& platform : worldPlatforms) {
        glPushMatrix();
        glTranslatef(platform.x, platform.y, platform.z);
        glColor3f(platform.r, platform.g, platform.b);
        
        float hw = platform.width / 2.0f;
        float hh = platform.height / 2.0f;
        float hd = platform.depth / 2.0f;
        
        glBegin(GL_QUADS);
        // Top face
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, hh, -hd);
        
        // Bottom face
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(-hw, -hh, hd);
        
        // Front face
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(-hw, hh, hd);
        
        // Back face
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, -hh, -hd);
        
        // Right face
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, -hh, hd);
        
        // Left face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(-hw, hh, -hd);
        glEnd();
        
        glPopMatrix();
    }
    
    // Draw cubes
    for (const Cube3D& cube : worldCubes) {
        glPushMatrix();
        glTranslatef(cube.x, cube.y, cube.z);
        glColor3f(cube.r, cube.g, cube.b);
        
        float hw = cube.width / 2.0f;
        float hh = cube.height / 2.0f;
        float hd = cube.depth / 2.0f;
        
        glBegin(GL_QUADS);
        // Front face
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(-hw, hh, hd);
        
        // Back face
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, -hh, -hd);
        
        // Top face
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, hh, -hd);
        
        // Bottom face
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(-hw, -hh, hd);
        
        // Right face
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, -hh, hd);
        
        // Left face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(-hw, hh, -hd);
        glEnd();
        
        glPopMatrix();
    }
    
    // Draw triangles
    for (const Triangle3D& triangle : worldTriangles) {
        glColor3f(triangle.r, triangle.g, triangle.b);
        glBegin(GL_TRIANGLES);
        
        // Calculate normal
        float v1x = triangle.x2 - triangle.x1;
        float v1y = triangle.y2 - triangle.y1;
        float v1z = triangle.z2 - triangle.z1;
        
        float v2x = triangle.x3 - triangle.x1;
        float v2y = triangle.y3 - triangle.y1;
        float v2z = triangle.z3 - triangle.z1;
        
        float nx = v1y * v2z - v1z * v2y;
        float ny = v1z * v2x - v1x * v2z;
        float nz = v1x * v2y - v1y * v2x;
        
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 1e-6) {
            nx /= len;
            ny /= len;
            nz /= len;
        }
        
        glNormal3f(nx, ny, nz);
        glVertex3f(triangle.x1, triangle.y1, triangle.z1);
        glVertex3f(triangle.x2, triangle.y2, triangle.z2);
        glVertex3f(triangle.x3, triangle.y3, triangle.z3);
        glEnd();
    }
}

void drawGround() {
    draw3DArena();
}

void setupArenaGeometry() {
    // Clear existing objects
    worldCubes.clear();
    worldTriangles.clear();
    worldPlatforms.clear();
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BUAT ARENA 3D DIBAWAH INI~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    // Platform utama sebagai starting area (di ground level y=0)
    // Y position = height/2 untuk menempatkan dasar platform tepat di Y=0
    addPlatform(0.0f, 0.25f, 0.0f, 10.0f, 10.0f, 0.5f, 0.7f, 0.7f, 0.7f);  // Main ground platform
    
    // Platform-platform dengan ketinggian yang wajar (tidak terlalu tinggi)
    addPlatform(15.0f, 0.5f, 0.0f, 6.0f, 6.0f, 1.0f, 0.8f, 0.6f, 0.6f);    // Platform tinggi 1.0
    addPlatform(-15.0f, 0.375f, 0.0f, 5.0f, 5.0f, 0.75f, 0.6f, 0.8f, 0.6f); // Platform tinggi 0.75
    addPlatform(0.0f, 0.625f, 20.0f, 4.0f, 4.0f, 1.25f, 0.6f, 0.6f, 0.8f);  // Platform tinggi 1.25
    addPlatform(-20.0f, 0.25f, 15.0f, 8.0f, 8.0f, 0.5f, 0.8f, 0.8f, 0.6f);  // Platform rendah
    
    // Kubus dengan ketinggian yang wajar (maksimal 0.8 unit agar bisa dicapai dengan maxJumpHeight = 1.0)
    addCube3D(5.0f, 0.4f, 5.0f, 2.0f, 0.8f, 2.0f, 1.0f, 0.2f, 0.2f);       // Kubus merah rendah
    addCube3D(-5.0f, 0.5f, -5.0f, 1.5f, 1.0f, 1.5f, 0.2f, 1.0f, 0.2f);     // Kubus hijau sedang
    addCube3D(10.0f, 0.375f, -8.0f, 1.0f, 0.75f, 1.0f, 0.2f, 0.2f, 1.0f);  // Kubus biru kecil
    addCube3D(15.0f, 1.25f, 0.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.2f);     // Kubus kuning di atas platform (0.5 + 1.0)
    
    // Kubus tinggi yang hanya bisa dicapai lewat ramp
    addCube3D(-8.0f, 1.0f, 8.0f, 1.5f, 2.0f, 1.5f, 1.0f, 0.5f, 0.0f);     // Kubus orange tinggi
    addCube3D(25.0f, 0.75f, 5.0f, 2.0f, 1.5f, 2.0f, 0.5f, 0.0f, 1.0f);    // Kubus purple tinggi
    
    // Ramp 3D segitiga siku-siku yang realistis
    // Ramp dari ground (y=0) ke platform tinggi (y=1.0)
    addTriangleRamp(10.0f, -5.0f,     // Start position (ground level)
                    14.0f, -1.0f,     // End position (dekat platform)
                    0.0f, 1.0f,       // Start height 0, End height 1.0
                    3.0f,             // Width ramp
                    0.2f, 1.0f, 0.2f); // Green ramp
    
    // Ramp menuju kubus orange tinggi
    addTriangleRamp(-12.0f, 5.0f,     // Start position
                    -8.0f, 8.0f,      // End position (dekat kubus orange)
                    0.0f, 2.0f,       // Start height 0, End height 2.0
                    2.5f,             // Width ramp
                    1.0f, 0.5f, 0.0f); // Orange ramp
    
    // Ramp menuju area tinggi
    addTriangleRamp(-2.0f, 15.0f,     // Start position
                    0.0f, 19.0f,      // End position (dekat platform tinggi)
                    0.0f, 1.25f,      // Start height 0, End height 1.25
                    4.0f,             // Width ramp
                    0.8f, 0.8f, 0.2f); // Yellow ramp
    
    // Platform jembatan penghubung (ketinggian sedang)
    addPlatform(7.5f, 0.4f, 5.0f, 4.0f, 6.0f, 0.3f, 0.5f, 0.5f, 0.5f);    // Jembatan 1
    addPlatform(-10.0f, 0.35f, 0.0f, 3.0f, 8.0f, 0.2f, 0.4f, 0.4f, 0.4f); // Jembatan 2
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BATAS PEMBUATAN ARENA 3D~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

#include "arena.h"
#include "globals.h" // Sekarang menyertakan GRID_SIZE, BOUNDS, defaultFallingHeight, pathBaseHeight, dll.
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
            } else { // axis == 'z'
                progress = (current_z_world - (centerZ - depth / 2.0f)) / depth;
            }
            progress = clamp(progress, 0.0f, 1.0f);
            arenaHeights[i][j] = startHeight + progress * (endHeight - startHeight);
        }
    }
}

// Fungsi baru untuk membuat kubus
// centerX, centerY, centerZ adalah pusat kubus
// sizeX, sizeY, sizeZ adalah dimensi kubus
void Kubus(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ) {
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    // Hitung batas-batas kubus dalam indeks grid
    // Untuk sumbu X
    int start_i = floor((centerX - sizeX / 2.0f + BOUNDS) / stepX);
    int end_i   = floor((centerX + sizeX / 2.0f + BOUNDS) / stepX);
    // Untuk sumbu Z
    int start_j = floor((centerZ - sizeZ / 2.0f + BOUNDS) / stepZ);
    int end_j   = floor((centerZ + sizeZ / 2.0f + BOUNDS) / stepZ);

    // Clamp indeks agar tetap dalam batas array arenaHeights
    start_i = clamp(start_i, 0, GRID_SIZE - 1);
    end_i   = clamp(end_i,   0, GRID_SIZE - 1);
    start_j = clamp(start_j, 0, GRID_SIZE - 1);
    end_j   = clamp(end_j,   0, GRID_SIZE - 1);

    // Ketinggian permukaan atas kubus
    float topSurfaceHeight = centerY + sizeY / 2.0f;

    for (int i = start_i; i <= end_i; ++i) {
        for (int j = start_j; j <= end_j; ++j) {

            arenaHeights[i][j] = topSurfaceHeight;
        }
    }
}

void setupArenaGeometry() {
    // 1. Inisialisasi seluruh arena ke ketinggian dasar (misalnya, area jatuh)
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            arenaHeights[i][j] = defaultFallingHeight; // defaultFallingHeight dari globals.h
        }
    }

    addFlatArea(0.0f, 0.0f, BOUNDS*2.0f, BOUNDS*2.0f, pathBaseHeight - 2.0f); // pathBaseHeight dari globals.h

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BUAT ARENA DIBAWAH INI~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    addRampArea(1.0f, 0.0f, 6.5f, 6.0f, pathBaseHeight - 2.0f, pathBaseHeight + 2.0f, 'z');
    addRampArea(-3.0f, 0.0f, 7.0f, 3.0f, pathBaseHeight + 3.0f, pathBaseHeight + 0.5f, 'x');
    Kubus(0.0f, pathBaseHeight + 1.0f, 0.0f, 5.0f, 1.0f, 10.0f); // Kubus di tengah arena
    Kubus(-6.0f, pathBaseHeight + 2.0f, 1.0f, 1.0f, 1.0f, 12.0f); 
    Kubus(-6.0f, pathBaseHeight + 1.0f, 13.0f, 10.0f, 1.0f, 2.0f); 
    Kubus(-6.0f, pathBaseHeight + 1.3f, 14.0f, 10.0f, 1.0f, 2.0f); //tadahan
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BATAS PEMBUATAN ARENA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

    i = clamp(i, 0, GRID_SIZE - 2); // Pastikan tidak keluar batas saat mengakses i+1
    j = clamp(j, 0, GRID_SIZE - 2); // Pastikan tidak keluar batas saat mengakses j+1

    float u = grid_x_float - i; // Fraksi antara grid i dan i+1
    float v = grid_z_float - j; // Fraksi antara grid j dan j+1

    // Interpolasi bilinear untuk mendapatkan ketinggian yang lebih halus
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

    // Hitung normal menggunakan metode beda hingga (finite differences)
    const float epsilon = 0.05f; // Jarak kecil untuk sampling
    float h_center = outHeight; // Ketinggian di titik (x,z) sudah dihitung
    float h_dx = getArenaHeight(x + epsilon, z); // Ketinggian di (x+eps, z)
    float h_dz = getArenaHeight(x, z + epsilon); // Ketinggian di (x, z+eps)


    outNormalX = (h_dx - h_center) * epsilon; // Komponen Y dari U * Komponen Z dari V (0*epsilon) - Komponen Z dari U * Komponen Y dari V (0*(h_dz-h_center)) -> ini salah
                 // Seharusnya: Uy*Vz - Uz*Vy = (h_dx - h_center) * epsilon - 0 * (h_dz - h_center)
    outNormalY = epsilon * epsilon; // Uz*Vx - Ux*Vz = 0*0 - epsilon*epsilon
    outNormalZ = -(h_dz - h_center) * epsilon; // Ux*Vy - Uy*Vx = epsilon*(h_dz-h_center) - (h_dx-h_center)*0


    outNormalX = -(h_dx - h_center); // Mengasumsikan dz adalah epsilon, ini adalah komponen dy untuk perubahan dx
    outNormalY = epsilon;            // Komponen "up"
    outNormalZ = -(h_dz - h_center); // Mengasumsikan dx adalah epsilon, ini adalah komponen dy untuk perubahan dz


    float normalLength = sqrt(outNormalX * outNormalX + outNormalY * outNormalY + outNormalZ * outNormalZ);
    if (normalLength > 1e-6) {
        outNormalX /= normalLength;
        outNormalY /= normalLength;
        outNormalZ /= normalLength;
    } else {
        // Jika panjang normal sangat kecil (permukaan sangat datar), default ke atas
        outNormalX = 0.0f;
        outNormalY = 1.0f;
        outNormalZ = 0.0f;
    }

    // Pastikan normal selalu mengarah ke atas (komponen Y positif)
    if (outNormalY < 0) {
        outNormalX *= -1;
        outNormalY *= -1;
        outNormalZ *= -1;
    }
}

// --- Drawing Functions ---
void drawGround() {
    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    // Set arena material properties for better lighting
    GLfloat arena_ambient[] = {0.2f, 0.2f, 0.25f, 1.0f};   // Cool ambient
    GLfloat arena_diffuse[] = {0.6f, 0.6f, 0.7f, 1.0f};    // Base arena color
    GLfloat arena_specular[] = {0.1f, 0.1f, 0.2f, 1.0f};   // Low specular for matte finish
    GLfloat arena_shininess = 10.0f;                        // Low shininess
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, arena_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, arena_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, arena_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, arena_shininess);

    // Definisikan warna untuk permukaan atas dan sisi
    // Warna sisi bisa dibuat lebih gelap atau berbeda
    float side_color_r = 0.25f; // Darker for sides
    float side_color_g = 0.25f;
    float side_color_b = 0.3f;

    // Threshold untuk perbedaan ketinggian yang menandakan "sisi"
    // Jika perbedaan ketinggian maksimum dalam satu quad melebihi nilai ini,
    // quad tersebut dianggap sebagai bagian dari sisi. Sesuaikan nilai ini jika perlu.
    float side_detection_threshold = 0.75f; // Misalnya, jika beda tinggi > 0.75 unit

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

            // Hitung perbedaan ketinggian maksimum di dalam quad
            float max_h_diff = 0.0f;
            max_h_diff = fmax(max_h_diff, fabs(y11 - y21)); // Tepi sepanjang X
            max_h_diff = fmax(max_h_diff, fabs(y12 - y22)); // Tepi sepanjang X lainnya
            max_h_diff = fmax(max_h_diff, fabs(y11 - y12)); // Tepi sepanjang Z
            max_h_diff = fmax(max_h_diff, fabs(y21 - y22)); // Tepi sepanjang Z lainnya

            bool is_side_quad = (max_h_diff > side_detection_threshold);

            // Hitung normal untuk pencahayaan (opsional tapi meningkatkan visual)
            // Vektor untuk dua sisi segitiga pertama dari quad: (P0, P1, P3) -> P0(x1,y11,z1), P1(x1,y12,z2), P3(x2,y21,z1)
            float v1x = 0;          float v1y = y12 - y11; float v1z = z2 - z1; // P1 - P0
            float v2x = x2 - x1;    float v2y = y21 - y11; float v2z = 0;       // P3 - P0
            
            float nx = v1y * v2z - v1z * v2y;
            float ny = v1z * v2x - v1x * v2z;
            float nz = v1x * v2y - v1y * v2x;

            // Normalisasi normal
            float len = sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 1e-6) { // Hindari pembagian dengan nol
                nx /= len;
                ny /= len;
                nz /= len;
            }
            glNormal3f(nx, ny, nz);            if (is_side_quad) {
                // Set darker material for sides with slightly more specular reflection
                GLfloat side_ambient[] = {0.15f, 0.15f, 0.2f, 1.0f};
                GLfloat side_diffuse[] = {side_color_r, side_color_g, side_color_b, 1.0f};
                GLfloat side_specular[] = {0.2f, 0.2f, 0.3f, 1.0f};
                GLfloat side_shininess = 20.0f;
                
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, side_ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, side_diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, side_specular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, side_shininess);
                
                glColor3f(side_color_r, side_color_g, side_color_b);
            } else {
                // Reset material for top surfaces
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, arena_ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, arena_diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, arena_specular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, arena_shininess);
                
                // Enhanced color calculation for permukaan atas with better lighting
                float avg_height = (y11 + y12 + y21 + y22) / 4.0f;
                float lowest_viz_height = defaultFallingHeight; 
                // Sesuaikan highest_viz_height agar rentang warna pada permukaan atas terlihat bagus
                float highest_viz_height = pathBaseHeight + 3.0f; // Contoh, bisa disesuaikan
                
                float color_factor = (avg_height - lowest_viz_height) / (highest_viz_height - lowest_viz_height);
                color_factor = clamp(color_factor, 0.0f, 1.0f);

                // Enhanced color scheme with better contrast for lighting
                float r = 0.3f + color_factor * 0.5f;  // Green to yellow-brown transition
                float g = 0.6f - color_factor * 0.2f;  // Maintain some green
                float b = 0.2f + color_factor * 0.3f;  // Add warmth at higher elevations
                
                glColor3f(r, g, b);
            }

            glVertex3f(x1, y11, z1);
            glVertex3f(x1, y12, z2);
            glVertex3f(x2, y22, z2);
            glVertex3f(x2, y21, z1);
        }
    }
    glEnd();
}
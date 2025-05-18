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
            // Set ketinggian grid di bawah kubus ke permukaan atas kubus
            // Ini akan membuat bola menggelinding di atasnya dan "terhalang" oleh sisinya
            // jika area di sekitarnya lebih rendah.
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

    // 2. Hapus pembuatan arena lama (panggilan ke addFlatArea dan addRampArea yang sebelumnya ada di sini)
    //    Arena sekarang akan dibangun menggunakan fungsi Kubus atau fungsi kustom lainnya.

    // 3. Contoh penggunaan fungsi Kubus:
    // Membuat platform awal yang datar sebagai alas
    addFlatArea(0.0f, 0.0f, BOUNDS*2.0f, BOUNDS*2.0f, pathBaseHeight - 2.0f); // pathBaseHeight dari globals.h

    // Kubus pertama di tengah, sedikit terangkat
    Kubus(0.0f, pathBaseHeight + 1.0f, 0.0f, 5.0f, 2.0f, 5.0f);

    // // Kubus kedua sebagai dinding di satu sisi
    // Kubus(8.0f, pathBaseHeight + 2.0f, 0.0f, 2.0f, 4.0f, 10.0f);

    // // Kubus ketiga lebih kecil dan lebih tinggi
    // Kubus(-5.0f, pathBaseHeight + 3.0f, -8.0f, 3.0f, 6.0f, 3.0f);~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Anda bisa menambahkan lebih banyak panggilan ke Kubus() atau fungsi pembuatan arena lainnya di sini
    // untuk membangun level yang lebih kompleks.
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

    // Vektor tangen pertama (arah x): (epsilon, h_dx - h_center, 0)
    // Vektor tangen kedua (arah z): (0, h_dz - h_center, epsilon)
    // Normal adalah hasil cross product dari tangen_z X tangen_x (atau sebaliknya dengan penyesuaian tanda)
    // (TangenZ.y * TangenX.z - TangenZ.z * TangenX.y) -> (h_dz - h_center) * 0 - epsilon * 0 = 0 (ini salah)
    // Perhitungan normal yang lebih umum:
    // Vektor P1 = (x, h_center, z)
    // Vektor P2 = (x + epsilon, h_dx, z)
    // Vektor P3 = (x, h_dz, z + epsilon)
    // Vektor U = P2 - P1 = (epsilon, h_dx - h_center, 0)
    // Vektor V = P3 - P1 = (0, h_dz - h_center, epsilon)
    // Normal N = U x V
    outNormalX = (h_dx - h_center) * epsilon; // Komponen Y dari U * Komponen Z dari V (0*epsilon) - Komponen Z dari U * Komponen Y dari V (0*(h_dz-h_center)) -> ini salah
                 // Seharusnya: Uy*Vz - Uz*Vy = (h_dx - h_center) * epsilon - 0 * (h_dz - h_center)
    outNormalY = epsilon * epsilon; // Uz*Vx - Ux*Vz = 0*0 - epsilon*epsilon
    outNormalZ = -(h_dz - h_center) * epsilon; // Ux*Vy - Uy*Vx = epsilon*(h_dz-h_center) - (h_dx-h_center)*0

    // Koreksi perhitungan normal (dari kode sebelumnya yang tampaknya lebih standar untuk heightmap)
    // Normal dari (P(x+dx,z) - P(x,z)) x (P(x,z+dz) - P(x,z))
    // (dx, height(x+dx,z)-height(x,z), 0)
    // (0, height(x,z+dz)-height(x,z), dz)
    // Nx = (height(x+dx,z)-height(x,z)) * dz
    // Ny = dx*dz (jika dx dan dz adalah epsilon)
    // Nz = -(height(x,z+dz)-height(x,z)) * dx

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
    // Warna dasar tanah bisa diatur di sini atau per-quad
    // glColor3f(0.2f, 0.7f, 0.2f); // Contoh warna hijau

    float stepX = (2.0f * BOUNDS) / (GRID_SIZE - 1);
    float stepZ = (2.0f * BOUNDS) / (GRID_SIZE - 1);

    glBegin(GL_QUADS);
    for (int i = 0; i < GRID_SIZE - 1; ++i) {
        for (int j = 0; j < GRID_SIZE - 1; ++j) {
            float x1 = -BOUNDS + (float)i * stepX;
            float z1 = -BOUNDS + (float)j * stepZ;
            float x2 = -BOUNDS + (float)(i + 1) * stepX;
            float z2 = -BOUNDS + (float)(j + 1) * stepZ;

            // Ambil ketinggian dari array (bukan dari getArenaHeight secara langsung untuk efisiensi drawing)
            float y11 = arenaHeights[i][j];
            float y21 = arenaHeights[i + 1][j];
            float y22 = arenaHeights[i + 1][j + 1];
            float y12 = arenaHeights[i][j + 1];

            // Pewarnaan berdasarkan ketinggian rata-rata quad
            float avg_height = (y11 + y12 + y21 + y22) / 4.0f;
            // Batas visual untuk pewarnaan (bisa disesuaikan atau diambil dari globals.h)
            float lowest_viz_height = defaultFallingHeight; // Ketinggian terendah yang terlihat
            float highest_viz_height = pathBaseHeight + 5.0f; // Perkiraan ketinggian tertinggi yang mungkin untuk pewarnaan
            float height_range = highest_viz_height - lowest_viz_height;
            if (height_range <= 0) height_range = 1.0f; // Hindari pembagian dengan nol

            float color_factor = clamp((avg_height - lowest_viz_height) / height_range, 0.0f, 1.0f);

            // Skema warna (misalnya, dari coklat ke hijau muda)
            glColor3f(
                0.4f - color_factor * 0.2f, // Merah: lebih sedikit saat lebih tinggi
                0.3f + color_factor * 0.4f, // Hijau: lebih banyak saat lebih tinggi
                0.2f                        // Biru: konstan atau sedikit variasi
            );

            // Gambar quad
            // Normal bisa dihitung per-vertex untuk shading yang lebih baik jika diinginkan,
            // tapi untuk sekarang kita gunakan pewarnaan flat per-quad.
            // Untuk shading Gouraud/Phong, Anda perlu menghitung normal di setiap vertex.
            // glNormal3f(...); // Jika menggunakan lighting per vertex

            glVertex3f(x1, y11, z1);
            glVertex3f(x1, y12, z2); // Urutan vertex penting untuk orientasi permukaan
            glVertex3f(x2, y22, z2);
            glVertex3f(x2, y21, z1);
        }
    }
    glEnd();
}
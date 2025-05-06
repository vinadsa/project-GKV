#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Variabel untuk rotasi
float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;

// Variabel untuk objek
typedef struct {
    float rotX, rotY, rotZ;  // Kecepatan rotasi
    float posX, posY, posZ;  // Posisi
    float r, g, b;           // Warna
    float size;              // Ukuran
} Cube;

#define NUM_CUBES 5
Cube cubes[NUM_CUBES];

// Fungsi untuk membuat kubus
void drawCube(float size, float r, float g, float b) {
    glColor3f(r, g, b);
    
    // Sisi Depan
    glBegin(GL_QUADS);
        glVertex3f(-size, -size, size);
        glVertex3f(size, -size, size);
        glVertex3f(size, size, size);
        glVertex3f(-size, size, size);
    glEnd();
    
    // Sisi Belakang
    glBegin(GL_QUADS);
        glColor3f(r*0.8, g*0.8, b*0.8);
        glVertex3f(-size, -size, -size);
        glVertex3f(-size, size, -size);
        glVertex3f(size, size, -size);
        glVertex3f(size, -size, -size);
    glEnd();
    
    // Sisi Atas
    glBegin(GL_QUADS);
        glColor3f(r*0.9, g*0.9, b*0.9);
        glVertex3f(-size, size, -size);
        glVertex3f(-size, size, size);
        glVertex3f(size, size, size);
        glVertex3f(size, size, -size);
    glEnd();
    
    // Sisi Bawah
    glBegin(GL_QUADS);
        glColor3f(r*0.7, g*0.7, b*0.7);
        glVertex3f(-size, -size, -size);
        glVertex3f(size, -size, -size);
        glVertex3f(size, -size, size);
        glVertex3f(-size, -size, size);
    glEnd();
    
    // Sisi Kanan
    glBegin(GL_QUADS);
        glColor3f(r*0.85, g*0.85, b*0.85);
        glVertex3f(size, -size, -size);
        glVertex3f(size, size, -size);
        glVertex3f(size, size, size);
        glVertex3f(size, -size, size);
    glEnd();
    
    // Sisi Kiri
    glBegin(GL_QUADS);
        glColor3f(r*0.75, g*0.75, b*0.75);
        glVertex3f(-size, -size, -size);
        glVertex3f(-size, -size, size);
        glVertex3f(-size, size, size);
        glVertex3f(-size, size, -size);
    glEnd();
}

// Fungsi untuk menggambar objek
void display() {
    // Membersihkan buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Reset transformasi
    glLoadIdentity();
    
    // Atur posisi kamera (mundur 10 unit)
    gluLookAt(0.0, 0.0, 10.0,  // Posisi kamera
              0.0, 0.0, 0.0,   // Titik yang dilihat
              0.0, 1.0, 0.0);  // Vektor up
    
    // Rotasi keseluruhan adegan
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    
    // Gambar kubus-kubus
    for (int i = 0; i < NUM_CUBES; i++) {
        glPushMatrix();
        
        // Posisikan kubus
        glTranslatef(cubes[i].posX, cubes[i].posY, cubes[i].posZ);
        
        // Rotasi individual untuk kubus
        glRotatef(rotationX * cubes[i].rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY * cubes[i].rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(rotationZ * cubes[i].rotZ, 0.0f, 0.0f, 1.0f);
        
        // Gambar kubus
        drawCube(cubes[i].size, cubes[i].r, cubes[i].g, cubes[i].b);
        
        glPopMatrix();
    }
    
    // Gambar kubus pusat (lebih besar)
    glPushMatrix();
    glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);
    drawCube(0.7f, 1.0f, 1.0f, 1.0f);  // Kubus putih di tengah
    glPopMatrix();
    
    // Tukar buffer (double buffering)
    glutSwapBuffers();
}

// Fungsi untuk mengatur perspektif saat ukuran jendela berubah
void reshape(int width, int height) {
    // Hindari pembagian dengan nol
    if (height == 0) height = 1;
    
    // Atur viewport sesuai ukuran jendela
    glViewport(0, 0, width, height);
    
    // Atur proyeksi perspektif
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Sudut pandang 45 derajat, rasio aspek, jarak dekat, jarak jauh
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    
    // Kembali ke mode modelview
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Fungsi untuk animasi
void animate() {
    // Perbarui rotasi
    rotationX += 0.3f;
    rotationY += 0.5f;
    rotationZ += 0.2f;
    
    // Perbarui tampilan
    glutPostRedisplay();
}

// Inisialisasi
void init() {
    // Seed untuk nomor acak
    srand(time(NULL));
    
    // Inisialisasi kubus-kubus
    for (int i = 0; i < NUM_CUBES; i++) {
        // Posisi acak dalam orbit
        float angle = ((float)i / NUM_CUBES) * 2.0f * 3.14159f;
        float radius = 2.5f;
        cubes[i].posX = cos(angle) * radius;
        cubes[i].posY = sin(angle) * radius;
        cubes[i].posZ = 0;
        
        // Kecepatan rotasi acak
        cubes[i].rotX = (float)(rand() % 10) / 10.0f + 0.5f;
        cubes[i].rotY = (float)(rand() % 10) / 10.0f + 0.5f;
        cubes[i].rotZ = (float)(rand() % 10) / 10.0f + 0.5f;
        
        // Warna acak
        cubes[i].r = (float)(rand() % 10) / 10.0f;
        cubes[i].g = (float)(rand() % 10) / 10.0f;
        cubes[i].b = (float)(rand() % 10) / 10.0f;
        
        // Ukuran acak
        cubes[i].size = (float)(rand() % 5 + 5) / 20.0f;  // 0.25 - 0.5
    }
    
    // Aktifkan Z-Buffer
    glEnable(GL_DEPTH_TEST);
    
    // Atur warna background
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);  // Biru gelap
}

// Program Utama
int main(int argc, char **argv) {
    // Inisialisasi GLUT
    glutInit(&argc, argv);
    
    // Atur mode tampilan (double buffer, RGBA, dengan depth buffer)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    // Ukuran jendela
    glutInitWindowSize(800, 600);
    
    // Posisi jendela
    glutInitWindowPosition(100, 100);
    
    // Buat jendela
    glutCreateWindow("Animasi 3D Kubus Berputar");
    
    // Registrasi fungsi callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(animate);
    
    // Inisialisasi
    init();
    
    // Loop utama GLUT
    glutMainLoop();
    
    return 0;
}
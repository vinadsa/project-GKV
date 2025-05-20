#include "graphics.h"
#include "globals.h"
#include "arena.h"    // For drawGround, getArenaHeightAndNormal
#include "marble.h"   // For drawMarble, marbleX, marbleZ (accessing via globals)
#include "camera.h"   // For camera variables (accessing via globals)
#include "physics.h"  // For updatePhysics
#include "utils.h"    // For degToRad
#include "checkpoint.h" // For drawCheckpoints
#include <GL/glut.h>
#include <GL/glu.h>   // For gluSphere, GLUquadric, gluBuild2DMipmaps
#include <cmath>     // For cos, sin
#include <iostream>  // For std::cout, std::cerr

// #define STB_IMAGE_IMPLEMENTATION // Define this in exactly one .c or .cpp file
#include "stb_image.h"         // For loading images

// Global definitions
GLuint marbleTextureID = 0; // Initialize to 0 (no texture)
GLUquadric* sphereQuadric = nullptr;

// Function to load a texture
bool loadTexture(const char* filename, GLuint& textureID_ref) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << " - " << stbi_failure_reason() << std::endl;
        return false;
    }

    glGenTextures(1, &textureID_ref);
    glBindTexture(GL_TEXTURE_2D, textureID_ref);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB;
    if (channels == 4) {
        format = GL_RGBA;
    } else if (channels == 3) {
        format = GL_RGB;
    } else {
        std::cerr << "Unsupported image format (channels: " << channels << ") for " << filename << std::endl;
        stbi_image_free(data);
        glDeleteTextures(1, &textureID_ref); // Clean up generated texture ID
        textureID_ref = 0;
        return false;
    }

    // Use gluBuild2DMipmaps for compatibility with older OpenGL contexts often used with GLUT
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    std::cout << "Texture loaded successfully: " << filename << std::endl;
    return true;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera target (look-at point)
    float targetX = marbleX;
    // Use marbleY directly for the target's Y position, so the camera follows the marble in the air.
    float targetY = marbleY + cameraTargetYOffset; // MODIFIED LINE
    float targetZ = marbleZ;

    // Calculate camera's eye position (eyeX, eyeY, eyeZ)
    float camAngleXRad = degToRad(cameraAngleX);
    float camAngleYRad = degToRad(cameraAngleY);

    // Corrected camera position calculation based on spherical coordinates
    // Assuming cameraDistance is the distance from the target point to the camera.
    // The camera orbits around the target point.
    float eyeX = targetX + cameraDistance * cos(camAngleYRad) * sin(camAngleXRad);
    float eyeY = targetY + cameraDistance * sin(camAngleYRad);
    float eyeZ = targetZ + cameraDistance * cos(camAngleYRad) * cos(camAngleXRad);

    // Ensure camera doesn't go below a certain height relative to the target or absolute minimum
    // This can prevent clipping into the marble or ground if cameraAngleY is too steep.
    // Example: if (eyeY < targetY + 0.2f) eyeY = targetY + 0.2f; // Keep camera slightly above target Y
    // Example: if (eyeY < 0.1f) eyeY = 0.1f; // Absolute minimum camera height


    gluLookAt(eyeX, eyeY, eyeZ,
              targetX, targetY, targetZ,
              0.0, 1.0, 0.0); // Up vector

    drawGround();
    drawMarble();
    drawCheckpoints();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int value) {
    updatePhysics();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void initGraphics() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    // If GL_COLOR_MATERIAL is enabled, glColor3f(1,1,1) should be set before drawing
    // textured objects for the texture to appear with its original colors.

    GLfloat light_pos[] = {0.0f, 30.0f, 30.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    GLfloat diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Sky blue

    // Load marble texture
    // Ensure the path is correct relative to your executable's working directory
    // or use an absolute path if necessary.
    // If your executable runs from project_GKV/src, then "textures/marble_texture.png" is correct.
    // If it runs from project_GKV, then "src/textures/marble_texture.png" would be needed.
    // For simplicity, assuming it runs from where graphics.cpp can see "textures/marble_texture.png"
    if (!loadTexture("textures/marble_texture.png", marbleTextureID)) {
        std::cerr << "Marble texture not loaded. Marble will be untextured." << std::endl;
    }

    // Initialize quadric for sphere
    sphereQuadric = gluNewQuadric();
    if (sphereQuadric) {
        gluQuadricNormals(sphereQuadric, GLU_SMOOTH);   // Generate smooth normals
        gluQuadricTexture(sphereQuadric, GL_TRUE);      // Generate texture coordinates
    } else {
        std::cerr << "Failed to create GLUquadric object." << std::endl;
    }
}
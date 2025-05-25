#include "graphics.h"
#include "globals.h"
#include "arena.h"    // For drawGround, getArenaHeightAndNormal
#include "marble.h"   // For drawMarble, marbleX, marbleZ (accessing via globals)
#include "camera.h"   // For camera variables (accessing via globals)
#include "physics.h"  // For updatePhysics
#include "utils.h"    // For degToRad
#include "checkpoint.h" // For drawCheckpoints
#include "timer.h"      // Added for timer functionality
#include <GL/glut.h>
#include <GL/glu.h>   // For gluSphere, GLUquadric, gluBuild2DMipmaps
#include <cmath>     // For cos, sin
#include <iostream>  // For std::cout, std::cerr

// #define STB_IMAGE_IMPLEMENTATION // Define this in exactly one .c or .cpp file
#include "stb_image.h"         // For loading images

// Global definitions
GLuint marbleTextureID = 0; // Initialize to 0 (no texture)
GLUquadric* sphereQuadric = nullptr;

// Dynamic lighting function that updates light positions based on marble position
void updateDynamicLighting() {
    // Update rim light to follow marble for better highlighting
    GLfloat light2_pos[] = {marbleX, marbleY + 15.0f, marbleZ - 20.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
      // Optional: Add subtle movement to fill light for more dynamic shadows
    static float lightTime = 0.0f;
    lightTime += 0.01f;
    GLfloat light1_pos[] = {
        -30.0f + 10.0f * sinf(lightTime * 0.5f), 
        40.0f + 5.0f * cosf(lightTime * 0.3f), 
        -20.0f + 8.0f * sinf(lightTime * 0.4f), 
        1.0f
    };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    
    // Add subtle intensity variation to make lighting more dynamic
    static float intensityTime = 0.0f;
    intensityTime += 0.005f;
      // Vary the main light intensity slightly for a more natural feel
    float intensityVariation = 0.9f + 0.1f * sinf(intensityTime);
    GLfloat light0_diffuse[] = {
        1.0f * intensityVariation, 
        0.95f * intensityVariation, 
        0.8f * intensityVariation, 
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
}

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

    // Update dynamic lighting based on marble position
    updateDynamicLighting();

    drawGround();
    drawMarble();
    drawCheckpoints();

    // Display the timer
    int screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    int screenHeight = glutGet(GLUT_WINDOW_HEIGHT);
    displayTimer(screenWidth, screenHeight);

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
    updateTimer(); // Add this line to update the timer every frame
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void initGraphics() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Enable smooth shading for better lighting quality
    glShadeModel(GL_SMOOTH);
    
    // Set global ambient light
    GLfloat globalAmbient[] = {0.3f, 0.3f, 0.4f, 1.0f}; // Slightly blue ambient
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // Two-sided lighting
    
    // Main directional light (sun-like)
    glEnable(GL_LIGHT0);
    GLfloat light0_pos[] = {50.0f, 80.0f, 30.0f, 0.0f}; // Directional light (w=0)
    GLfloat light0_ambient[] = {0.2f, 0.2f, 0.3f, 1.0f}; // Cool ambient
    GLfloat light0_diffuse[] = {1.0f, 0.95f, 0.8f, 1.0f}; // Warm sunlight
    GLfloat light0_specular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // White specular
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    
    // Secondary fill light (point light for softer shadows)
    glEnable(GL_LIGHT1);
    GLfloat light1_pos[] = {-30.0f, 40.0f, -20.0f, 1.0f}; // Point light (w=1)
    GLfloat light1_ambient[] = {0.1f, 0.1f, 0.15f, 1.0f};
    GLfloat light1_diffuse[] = {0.4f, 0.5f, 0.7f, 1.0f}; // Cool fill light
    GLfloat light1_specular[] = {0.3f, 0.3f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    
    // Set light attenuation for point light
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.002f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);
    
    // Rim light for marble highlighting
    glEnable(GL_LIGHT2);
    GLfloat light2_pos[] = {0.0f, 20.0f, -50.0f, 1.0f}; // Behind and above
    GLfloat light2_ambient[] = {0.05f, 0.05f, 0.1f, 1.0f};
    GLfloat light2_diffuse[] = {0.3f, 0.4f, 0.6f, 1.0f}; // Cool rim light
    GLfloat light2_specular[] = {0.8f, 0.9f, 1.0f, 1.0f}; // Bright specular
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
    
    // Enable automatic normal normalization (important for scaled objects)
    glEnable(GL_NORMALIZE);
    
    // Add fog for atmospheric effect
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.4f, 0.7f, 0.9f, 1.0f}; // Match sky color
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 50.0f);  // Fog starts at distance 50
    glFogf(GL_FOG_END, 200.0f);   // Fog is fully opaque at distance 200
    glFogf(GL_FOG_DENSITY, 0.02f); // Fog density (used with GL_EXP/GL_EXP2)
    
    glClearColor(0.4f, 0.7f, 0.9f, 1.0f); // Slightly darker sky blue for better contrast

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
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
#include <string> // For std::string
// #define STB_IMAGE_IMPLEMENTATION // Define this in exactly one .c or .cpp file
#include "stb_image.h"         // For loading images

// Global definitions
GLuint marbleTextureID = 0; // Initialize to 0 (no texture)
GLUquadric* sphereQuadric = nullptr;


void drawScore() {
    // Calculate total possible score based on collected checkpoints
    // Count uncollected checkpoints (excluding first checkpoint which is spawn point)
    int totalPossibleScore = 0;
    if (checkpoints.size() > 1) { // If there's more than just the spawn checkpoint
        for (int i = 1; i < checkpoints.size(); ++i) { // Start from index 1 to exclude spawn
            totalPossibleScore += 100; // Each checkpoint is worth 100 points
        }
    }
    
    std::string scoreStr = "Score: " + std::to_string(score) + "/" + std::to_string(totalPossibleScore);
    glColor3f(1.0f, 1.0f, 0.0f); // Kuning
    void* font = GLUT_BITMAP_HELVETICA_18;

    // Setup ortho projection supaya text selalu di posisi layar
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int textWidth = scoreStr.length() * 10; // Kira-kira, tergantung font
    int x = width / 2 - textWidth / 2;
    int y = height - 40;

    glRasterPos2i(x, y);
    for (char c : scoreStr) {
        glutBitmapCharacter(font, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

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
      // Vary the main light intensity slightly for a more natural feel (reduced variation)
    float intensityVariation = 0.95f + 0.05f * sinf(intensityTime); // Much smaller variation
    GLfloat light0_diffuse[] = {
        0.7f * intensityVariation, 
        0.65f * intensityVariation, 
        0.6f * intensityVariation, 
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

// Replace glShadowProjection with robust version for a plane and point light
void glShadowProjection(const float* light, const float* plane) {
    float dot = plane[0]*light[0] + plane[1]*light[1] + plane[2]*light[2] + plane[3]*light[3];
    float mat[16];
    mat[0]  = dot - light[0]*plane[0];
    mat[4]  = 0.0f - light[0]*plane[1];
    mat[8]  = 0.0f - light[0]*plane[2];
    mat[12] = 0.0f - light[0]*plane[3];

    mat[1]  = 0.0f - light[1]*plane[0];
    mat[5]  = dot - light[1]*plane[1];
    mat[9]  = 0.0f - light[1]*plane[2];
    mat[13] = 0.0f - light[1]*plane[3];

    mat[2]  = 0.0f - light[2]*plane[0];
    mat[6]  = 0.0f - light[2]*plane[1];
    mat[10] = dot - light[2]*plane[2];
    mat[14] = 0.0f - light[2]*plane[3];

    mat[3]  = 0.0f - light[3]*plane[0];
    mat[7]  = 0.0f - light[3]*plane[1];
    mat[11] = 0.0f - light[3]*plane[2];
    mat[15] = dot - light[3]*plane[3];

    glMultMatrixf(mat);
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
    float eyeZ = targetZ + cameraDistance * cos(camAngleYRad) * cos(camAngleXRad);    // Ensure camera doesn't go below a certain height relative to the target or absolute minimum
    // This can prevent clipping into the marble or ground if cameraAngleY is too steep.
    if (eyeY < targetY + 0.2f) eyeY = targetY + 0.2f; // Keep camera slightly above target Y
    if (eyeY < 0.1f) eyeY = 0.1f; // Absolute minimum camera heightY < 0.1f) eyeY = 0.1f; // Absolute minimum camera height
    gluLookAt(eyeX, eyeY, eyeZ,
              targetX, targetY, targetZ,
              0.0, 1.0, 0.0); // Up vector

    // Update dynamic lighting based on marble position
    updateDynamicLighting();

    drawGround();


    // Draw Marble's Shadow (real shadow projection)
    glPushMatrix();
        // Set up robust shadow projection matrix
        GLfloat shadow_plane[4] = {0.0f, 1.0f, 0.0f, -0.01f}; // y=0.01 plane to avoid z-fighting
        GLfloat shadow_light[4] = {10.0f, 80.0f, 10.0f, 1.0f}; // w=1 for point light
        glShadowProjection(shadow_light, shadow_plane);
        glTranslatef(marbleX, marbleY, marbleZ); // Use real marble position

        // --- Shadow artifact fix: polygon offset and depth mask ---
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -2.0f);
        glDepthMask(GL_FALSE);

        glDisable(GL_LIGHTING); // Only disable lighting for the shadow
        glColor4f(0.1f, 0.1f, 0.1f, 0.5f); // Shadow color, semi-transparent
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (sphereQuadric) {
            gluSphere(sphereQuadric, 0.5f, 24, 16);
        }
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING); // Re-enable lighting for the rest of the scene

        // --- Restore OpenGL state ---
        glDepthMask(GL_TRUE);
        glDisable(GL_POLYGON_OFFSET_FILL);
    glPopMatrix();


    // Draw actual objects
    drawMarble(); // The actual marble
    drawCheckpoints();
    drawScore(); // Draw the score on the screen

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
    
    // Check if countdown timer has expired
    if (isCountdownExpired()) {
        std::cout << "Time's up! Game Over!" << std::endl;
        // Optionally reset the game or show game over screen
        // For now, we'll just restart the game
        initGame();
    }
    
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
      // Set global ambient light (reduced to preserve texture visibility)
    GLfloat globalAmbient[] = {0.15f, 0.15f, 0.2f, 1.0f}; // Lower ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // Two-sided lighting
    
    // Main directional light (sun-like) - reduced intensity
    glEnable(GL_LIGHT0);
    GLfloat light0_pos[] = {50.0f, 80.0f, 30.0f, 0.0f}; // Directional light (w=0)
    GLfloat light0_ambient[] = {0.1f, 0.1f, 0.1f, 1.0f}; // Reduced ambient
    GLfloat light0_diffuse[] = {0.7f, 0.65f, 0.6f, 1.0f}; // Reduced warm sunlight
    GLfloat light0_specular[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Reduced specular
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    
    // Secondary fill light (point light for softer shadows) - reduced intensity
    glEnable(GL_LIGHT1);
    GLfloat light1_pos[] = {-30.0f, 40.0f, -20.0f, 1.0f}; // Point light (w=1)
    GLfloat light1_ambient[] = {0.05f, 0.05f, 0.08f, 1.0f};
    GLfloat light1_diffuse[] = {0.25f, 0.3f, 0.4f, 1.0f}; // Reduced cool fill light
    GLfloat light1_specular[] = {0.2f, 0.2f, 0.25f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    
    // Set light attenuation for point light
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.002f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);
    
    // Rim light for marble highlighting - much more subtle
    glEnable(GL_LIGHT2);
    GLfloat light2_pos[] = {0.0f, 20.0f, -50.0f, 1.0f}; // Behind and above
    GLfloat light2_ambient[] = {0.02f, 0.02f, 0.05f, 1.0f};
    GLfloat light2_diffuse[] = {0.15f, 0.2f, 0.3f, 1.0f}; // Much more subtle rim light
    GLfloat light2_specular[] = {0.4f, 0.45f, 0.5f, 1.0f}; // Reduced specular
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
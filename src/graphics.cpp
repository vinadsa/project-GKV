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

// Add the shadow projection function from the PDF
void glShadowProjection(float * l, float * e, float * n)
{
 float d, c;
 float mat[16];
 // d = N dot L
 d = n[0]*l[0] + n[1]*l[1] + n[2]*l[2];
 // c = (E dot N) - d
 c = e[0]*n[0] + e[1]*n[1] + e[2]*n[2] - d;

 // Create the projection matrix
 mat[0] = l[0]*n[0]+c; 
 mat[4] = n[1]*l[0];
 mat[8] = n[2]*l[0];
 mat[12] = -l[0]*c-l[0]*d;

 mat[1] = n[0]*l[1];
 mat[5] = l[1]*n[1]+c;
 mat[9] = n[2]*l[1];
 mat[13] = -l[1]*c-l[1]*d;

 mat[2] = n[0]*l[2];
 mat[6] = n[1]*l[2];
 mat[10] = l[2]*n[2]+c;
 mat[14] = -l[2]*c-l[2]*d;

 mat[3] = n[0];
 mat[7] = n[1];
 mat[11] = n[2];
 mat[15] = -d;

 // Multiply the current matrix by the projection matrix
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

    // Draw Marble's Shadow
    // Save relevant OpenGL states
    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);
    glPushMatrix();
        // Get current position of a light source for shadow casting (e.g., LIGHT0 or LIGHT1)
        // Using LIGHT0 as it's the main directional light in this setup.
        // The PDF example uses a global 'l'. Here we get it from an existing light.
        GLfloat current_light_pos[4];
        glGetLightfv(GL_LIGHT0, GL_POSITION, current_light_pos); 
        // For directional light (w=0), its position is a direction. For point light (w=1), it's a position.
        // glShadowProjection expects a position. If using directional light, this might need adjustment
        // or ensure 'l' in glShadowProjection is treated as a direction if light is directional.
        // For simplicity, let's assume current_light_pos is usable as 'l'.
        // The PDF's 'l' was {0, 80, 0}, a position. LIGHT0 is {50,80,30,0} a direction.
        // Let's use LIGHT1 as it's a point light { -30.0f, 40.0f, -20.0f, 1.0f } initially
        // and its position is updated by updateDynamicLighting.
        glGetLightfv(GL_LIGHT1, GL_POSITION, current_light_pos);


        GLfloat shadow_light_pos[3] = {current_light_pos[0], current_light_pos[1], current_light_pos[2]};
        
        // Point on the plane (ground is at y=0)
        GLfloat shadow_plane_point[3] = {0.0f, 0.0f, 0.0f}; // e.g., any point on y=0 plane
        
        // Normal of the plane (ground normal is 0,1,0)
        GLfloat shadow_plane_normal[3] = {0.0f, 1.0f, 0.0f};

        // Apply shadow projection matrix
        glShadowProjection(shadow_light_pos, shadow_plane_point, shadow_plane_normal);

        glDisable(GL_LIGHTING); // Shadows are not lit
        if (marbleTextureID != 0) { // If marble is textured
            glDisable(GL_TEXTURE_2D); // Shadows are typically untextured
        }
        glColor4f(0.1f, 0.1f, 0.1f, 0.5f); // Dark, semi-transparent shadow color

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Ensure this line is correct
        glDepthMask(GL_FALSE);                             // Ensure this line is correct

        // Stencil buffer setup to prevent shadow from drawing on itself or where it shouldn't
        // This is a common technique for planar shadows.
        // 1. Clear stencil buffer for the ground area (optional, if not cleared elsewhere)
        // glClear(GL_STENCIL_BUFFER_BIT); // Careful if other stencil ops are used
        // 2. Draw the ground (or ensure it's drawn before shadows)
        //    (drawGround() is already called before this block)
        // 3. Configure stencil test to draw shadow only on the ground
        //    This example assumes ground pixels have stencil value 0.
        //    If ground itself sets stencil values, adjust accordingly.
        glEnable(GL_STENCIL_TEST);
        // glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil before drawing shadow pass
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // Mark stencil pixels where shadow is drawn
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Replace stencil with 1
        // We draw the shadow, and it marks the stencil buffer.
        // To avoid z-fighting with the ground, an offset can be used, or ensure shadow is slightly offset.
        // The PDF example draws the "ground" quad slightly below the shadow projection plane point 'e'.
        // Here, both are at y=0. glPolygonOffset can help.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f); // Push shadow slightly away from camera to avoid z-fighting

        drawMarble(); // Draw the marble to cast its shadow

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_TRUE); // Restore depth mask
        glDisable(GL_BLEND);  // Restore blend
        
        // Texturing and Lighting will be restored by glPopAttrib
        
    glPopMatrix();
    glPopAttrib(); // Restore OpenGL states


    // Draw actual objects
    drawMarble(); // The actual marble
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
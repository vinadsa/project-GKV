#include "graphics.h"
#include "globals.h"
#include "arena.h"   
#include "marble.h"  
#include "camera.h"  
#include "physics.h"  
#include "utils.h"   
#include "checkpoint.h" 
#include "timer.h"     
#include "imageloader.h" 
#include <GL/glut.h>
#include <GL/glu.h>  
#include <cmath>   
#include <iostream>  
#include <string> 



GLuint marbleTextureID = 0;
GLUquadric* sphereQuadric = nullptr;
bool enableShadows = true; 


void drawScore() {
    int totalPossibleScore = 0;
    if (checkpoints.size() > 1) { 
        for (int i = 1; i < checkpoints.size(); ++i) { 
            totalPossibleScore += 100; 
        }
    }
    
    std::string scoreStr = "Score: " + std::to_string(score) + "/" + std::to_string(totalPossibleScore);
    glColor3f(1.0f, 1.0f, 0.0f);
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

void updateDynamicLighting() {
    GLfloat light2_pos[] = {marbleX, marbleY + 15.0f, marbleZ - 20.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    static float lightTime = 0.0f;
    lightTime += 0.01f;
    GLfloat light1_pos[] = {
        -30.0f + 10.0f * sinf(lightTime * 0.5f), 
        40.0f + 5.0f * cosf(lightTime * 0.3f), 
        -20.0f + 8.0f * sinf(lightTime * 0.4f), 
        1.0f
    };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    static float intensityTime = 0.0f;
    intensityTime += 0.005f;
    float intensityVariation = 0.95f + 0.05f * sinf(intensityTime);
    GLfloat light0_diffuse[] = {
        0.7f * intensityVariation, 
        0.65f * intensityVariation, 
        0.6f * intensityVariation, 
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
}

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

    float targetX = marbleX;
    float targetY = marbleY + cameraTargetYOffset;
    float targetZ = marbleZ;

    float camAngleXRad = degToRad(cameraAngleX);
    float camAngleYRad = degToRad(cameraAngleY);

    float eyeX = targetX + cameraDistance * cos(camAngleYRad) * sin(camAngleXRad);
    float eyeY = targetY + cameraDistance * sin(camAngleYRad);
    float eyeZ = targetZ + cameraDistance * cos(camAngleYRad) * cos(camAngleXRad);
    if (eyeY < targetY + 0.2f) eyeY = targetY + 0.2f;
    if (eyeY < 0.1f) eyeY = 0.1f; 
    gluLookAt(eyeX, eyeY, eyeZ,
              targetX, targetY, targetZ,
              0.0, 1.0, 0.0); 

    updateDynamicLighting();

    drawGround();


    if (enableShadows) {
        glPushMatrix();
            GLfloat shadow_plane[4] = {0.0f, 1.0f, 0.0f, -0.01f};
            GLfloat shadow_light[4] = {10.0f, 80.0f, 10.0f, 1.0f}; 
            glShadowProjection(shadow_light, shadow_plane);
            glTranslatef(marbleX, marbleY, marbleZ); 

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-2.0f, -2.0f);
            glDepthMask(GL_FALSE);

            glDisable(GL_LIGHTING); 
            glColor4f(0.1f, 0.1f, 0.1f, 0.5f); 
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (sphereQuadric) {
                gluSphere(sphereQuadric, 0.5f, 24, 16);
            }
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING); 

            glDepthMask(GL_TRUE);
            glDisable(GL_POLYGON_OFFSET_FILL);
        glPopMatrix();
    }


    drawMarble();
    drawCheckpoints();
    drawFinish(); 
    drawScore(); 
    drawCongratulationsPopup();

    int screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    int screenHeight = glutGet(GLUT_WINDOW_HEIGHT);
    displayTimer(screenWidth, screenHeight);

    drawCongratulationsPopup();

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
    updateTimer();
    
    if (isCountdownExpired()) {
        std::cout << "Time's up! Game Over!" << std::endl;
        initGame();
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); 
}

void initGraphics() {
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glShadeModel(GL_SMOOTH);
    GLfloat globalAmbient[] = {0.15f, 0.15f, 0.2f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glEnable(GL_LIGHT0);
    GLfloat light0_pos[] = {50.0f, 80.0f, 30.0f, 0.0f};
    GLfloat light0_ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat light0_diffuse[] = {0.7f, 0.65f, 0.6f, 1.0f};
    GLfloat light0_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    
    glEnable(GL_LIGHT1);
    GLfloat light1_pos[] = {-30.0f, 40.0f, -20.0f, 1.0f};
    GLfloat light1_ambient[] = {0.05f, 0.05f, 0.08f, 1.0f};
    GLfloat light1_diffuse[] = {0.25f, 0.3f, 0.4f, 1.0f};
    GLfloat light1_specular[] = {0.2f, 0.2f, 0.25f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.002f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);
    
    glEnable(GL_LIGHT2);
    GLfloat light2_pos[] = {0.0f, 20.0f, -50.0f, 1.0f};
    GLfloat light2_ambient[] = {0.02f, 0.02f, 0.05f, 1.0f};
    GLfloat light2_diffuse[] = {0.15f, 0.2f, 0.3f, 1.0f};
    GLfloat light2_specular[] = {0.4f, 0.45f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
    
    glEnable(GL_NORMALIZE);
    
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.4f, 0.7f, 0.9f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 50.0f);    glFogf(GL_FOG_END, 200.0f);    glFogf(GL_FOG_DENSITY, 0.02f);

    Image* image = loadBMP("textures/marble_texture.bmp"); 
    if (image == nullptr) {
        std::cerr << "Failed to load marble texture using imageloader." << std::endl;
    } else {
        glGenTextures(1, &marbleTextureID);
        glBindTexture(GL_TEXTURE_2D, marbleTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
        delete image;
        std::cout << "Marble texture loaded successfully using imageloader." << std::endl;
    }

    sphereQuadric = gluNewQuadric();
    if (sphereQuadric) {
        gluQuadricNormals(sphereQuadric, GLU_SMOOTH);
        gluQuadricTexture(sphereQuadric, GL_TRUE);
    } else {
        std::cerr << "Failed to create GLUquadric object." << std::endl;
    }
}

void drawCongratulationsPopup() {
    extern bool finishReached;
    if (!finishReached) return;
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    const char* lines[] = {"CONGRATULATIONS!", "You finished the level!"};
    int numLines = 2;
    char scoreLine[64];
    snprintf(scoreLine, sizeof(scoreLine), "Your Score: %d", score);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    int boxW = 420, boxH = 160;
    int boxX = width/2 - boxW/2, boxY = height/2 - boxH/2;
    glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
    glEnd();
    glColor3f(1.0f, 1.0f, 0.0f);
    void* font = GLUT_BITMAP_HELVETICA_18;
    int lineHeight = 36;
    // Tampilkan dua baris utama
    for (int i = 0; i < numLines; ++i) {
        int textWidth = glutBitmapLength(font, (const unsigned char*)lines[i]);
        int textX = width/2 - textWidth/2;
        int textY = height/2 + (numLines)*lineHeight/2 - i*lineHeight + 8;
        if (textX < boxX + 10) textX = boxX + 10;
        if (textX + textWidth > boxX + boxW - 10) textX = boxX + boxW - 10 - textWidth;
        if (textY < boxY + 20) textY = boxY + 20;
        if (textY > boxY + boxH - 20) textY = boxY + boxH - 20;
        glRasterPos2i(textX, textY);
        for (const char* p = lines[i]; *p; ++p) {
            glutBitmapCharacter(font, *p);
        }
    }
    // Tampilkan skor di bawahnya
    int scoreTextWidth = glutBitmapLength(font, (const unsigned char*)scoreLine);
    int scoreTextX = width/2 - scoreTextWidth/2;
    int scoreTextY = height/2 - (numLines)*lineHeight/2 + 8;
    if (scoreTextX < boxX + 10) scoreTextX = boxX + 10;
    if (scoreTextX + scoreTextWidth > boxX + boxW - 10) scoreTextX = boxX + boxW - 10 - scoreTextWidth;
    if (scoreTextY < boxY + 20) scoreTextY = boxY + 20;
    if (scoreTextY > boxY + boxH - 20) scoreTextY = boxY + boxH - 20;
    glColor3f(0.4f, 1.0f, 0.4f);
    glRasterPos2i(scoreTextX, scoreTextY);
    for (const char* p = scoreLine; *p; ++p) {
        glutBitmapCharacter(font, *p);
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
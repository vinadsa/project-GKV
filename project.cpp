#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <map> // Can be used for a more scalable approach

// Arena bounds
const float BOUNDS = 5.0f;

// Marble position
float marbleX = 0.0f, marbleZ = 0.0f;
float marbleVX = 0.0f, marbleVZ = 0.0f;

// Tilt rotation (control)
float tiltX = 0.0f, tiltZ = 0.0f;
const float maxTilt = 35.0f; // Max absolute tilt limit

// Gravity & friction
const float gravity = 9.8f;
const float friction = 0.98f; // Friction on ball speed
const float deltaTime = 0.016f; // ~60 FPS

// --- Camera Variables ---
float cameraAngleX = 0.0f;
float cameraAngleY = 30.0f;
float cameraDistance = 12.0f;
float cameraTargetYOffset = 0.5f;

// --- Mouse State Variables ---
bool isDragging = false;
int lastMouseX = 0, lastMouseY = 0;
const float mouseSensitivity = 0.4f;

// --- NEW Keyboard State Variables ---
bool keyStates[256]; // Array to store status of all keys (ASCII & Special)
					 // Or use map: std::map<int, bool> keyStates;
const float tiltIncrementSpeed = 30.0f; // Tilt change speed (degrees per second)
// Optional: Damping for tilt to return to 0 when key is released
// const float tiltDamping = 0.95f;


float degToRad(float deg) {
	return deg * 3.14159f / 180.0f;
}

float clamp(float value, float minVal, float maxVal) {
	if (value < minVal) return minVal;
	if (value > maxVal) return maxVal;
	return value;
}

float getGroundHeight(float x, float z) {
	float clampedX = clamp(x, -BOUNDS, BOUNDS);
	float clampedZ = clamp(z, -BOUNDS, BOUNDS);
	float angleX = degToRad(tiltX);
	float angleZ = degToRad(tiltZ);
	return tan(angleZ) * clampedX + tan(angleX) * clampedZ;
}

void drawMarble() {
	float groundH = getGroundHeight(marbleX, marbleZ);
	float marbleY = groundH + 0.5f;

	glPushMatrix();
	glTranslatef(marbleX, marbleY, marbleZ);
	glColor3f(1.0f, 0.1f, 0.1f);
	glutSolidSphere(0.5, 32, 32);
	glPopMatrix();
}

void drawGround() {
	glColor3f(0.2f, 0.7f, 0.2f);
	glBegin(GL_QUADS);
		glVertex3f(-BOUNDS, getGroundHeight(-BOUNDS, -BOUNDS), -BOUNDS);
		glVertex3f(-BOUNDS, getGroundHeight(-BOUNDS,  BOUNDS),  BOUNDS);
		glVertex3f( BOUNDS, getGroundHeight( BOUNDS,  BOUNDS),  BOUNDS);
		glVertex3f( BOUNDS, getGroundHeight( BOUNDS, -BOUNDS), -BOUNDS);
	glEnd();

	glColor3f(0.1f, 0.1f, 0.1f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	   glVertex3f(-BOUNDS, getGroundHeight(-BOUNDS, -BOUNDS), -BOUNDS);
	   glVertex3f(-BOUNDS, getGroundHeight(-BOUNDS,  BOUNDS),  BOUNDS);
	   glVertex3f( BOUNDS, getGroundHeight( BOUNDS,  BOUNDS),  BOUNDS);
	   glVertex3f( BOUNDS, getGroundHeight( BOUNDS, -BOUNDS), -BOUNDS);
	glEnd();
}

// --- updatePhysics function MODIFIED for bouncier collision ---
void updatePhysics() {
	float accX = -sin(degToRad(tiltZ)) * gravity;
	float accZ = -sin(degToRad(tiltX)) * gravity;

	marbleVX += accX * deltaTime;
	marbleVZ += accZ * deltaTime;

	marbleVX *= friction;
	marbleVZ *= friction;

	marbleX += marbleVX * deltaTime;
	marbleZ += marbleVZ * deltaTime;

	float radius = 0.5f;

	// Coefficient of restitution (0.0 = no bounce, 1.0 = perfect bounce)
	// Change this value to control the bounce strength
	const float restitution = 0.8f; // Value changed from 0.5f to 0.8f (try 0.9f or 1.0f too)

	if (marbleX < -BOUNDS + radius) {
		marbleX = -BOUNDS + radius; // Position exactly at the boundary
		marbleVX *= -restitution; // Reverse X velocity and multiply by restitution coefficient
	}
	if (marbleX >  BOUNDS - radius) {
		marbleX =  BOUNDS - radius;
		marbleVX *= -restitution;
	}
	if (marbleZ < -BOUNDS + radius) {
		marbleZ = -BOUNDS + radius;
		marbleVZ *= -restitution; // Reverse Z velocity and multiply by restitution coefficient
	}
	if (marbleZ >  BOUNDS - radius) {
		marbleZ =  BOUNDS - radius;
		marbleVZ *= -restitution;
	}
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	float targetX = marbleX;
	float targetY = getGroundHeight(marbleX, marbleZ) + cameraTargetYOffset;
	float targetZ = marbleZ;

	float camAngleXRad = degToRad(cameraAngleX);
	float camAngleYRad = degToRad(cameraAngleY);

	float camX = targetX + cameraDistance * cos(camAngleYRad) * sin(camAngleXRad);
	float camY = targetY + cameraDistance * sin(camAngleYRad);
	float camZ = targetZ + cameraDistance * cos(camAngleYRad) * cos(camAngleXRad);

	gluLookAt(camX, camY, camZ,
			  targetX, targetY, targetZ,
			  0.0, 1.0, 0.0);

	drawGround();
	drawMarble();

	glutSwapBuffers();
}


// --- NEW Function to process tilt input ---
void processTiltInput() {
	// 1. Get camera direction
	float camAngleXRad = degToRad(cameraAngleX);
	float cosCam = cos(camAngleXRad);
	float sinCam = sin(camAngleXRad);

	// 2. Relative direction vectors
	float forwardX = -sinCam;
	float forwardZ = -cosCam;
	float rightX = cosCam;
	float rightZ = -sinCam;

	// 3. Determine net tilt input based on key states
	float netTiltForward = 0.0f;
	float netTiltRight = 0.0f;

	if (keyStates[GLUT_KEY_UP]) 	netTiltForward += 1.0f;
	if (keyStates[GLUT_KEY_DOWN]) 	netTiltForward -= 1.0f;
	if (keyStates[GLUT_KEY_LEFT]) 	netTiltRight -= 1.0f; // Left tilt -> ball goes right (-right direction)
	if (keyStates[GLUT_KEY_RIGHT]) netTiltRight += 1.0f; // Right tilt -> ball goes left (+right direction)

	// 4. Calculate magnitude & normalize input vector
	float magnitude = sqrt(netTiltForward * netTiltForward + netTiltRight * netTiltRight);
	bool isInputActive = magnitude > 0.01f; // Check if any arrow key is pressed

	if (isInputActive) {
		float normForward = netTiltForward / magnitude;
		float normRight = netTiltRight / magnitude;

		// 5. Calculate tilt change for this frame
		float tiltIncrement = tiltIncrementSpeed * deltaTime; // Amount of tilt added this frame

		// Decompose:
		// Tilt change from Forward component
		float deltaTiltX_f = normForward * tiltIncrement * cosCam;
		float deltaTiltZ_f = normForward * tiltIncrement * sinCam;

		// Tilt change from Right component
		float deltaTiltX_r = normRight * tiltIncrement * sinCam;
		float deltaTiltZ_r = normRight * tiltIncrement * (-cosCam);

		// Apply total change
		tiltX += deltaTiltX_f + deltaTiltX_r;
		tiltZ += deltaTiltZ_f + deltaTiltZ_r;

		// Clamp absolute tilt value
		tiltX = clamp(tiltX, -maxTilt, maxTilt);
		tiltZ = clamp(tiltZ, -maxTilt, maxTilt);
	}
	// Optional: Return tilt to zero if no input
	/*
	else {
		// Only apply damping if tilt is not zero
		if (abs(tiltX) > 0.01f || abs(tiltZ) > 0.01f) {
			 tiltX *= tiltDamping;
			 tiltZ *= tiltDamping;
			 // Stop damping if very close to zero
			 if (abs(tiltX) < 0.01f) tiltX = 0.0f;
			 if (abs(tiltZ) < 0.01f) tiltZ = 0.0f;
		}
	}
	*/
}


void timer(int value) {
	// --- CALL Process Tilt Input Here ---
	processTiltInput();

	updatePhysics();
	glutPostRedisplay();
	glutTimerFunc(16, timer, 0);
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

// --- MODIFIED Keyboard Callbacks ---

// Called when a SPECIAL key is pressed (Arrow, F1-F12, etc.)
void specialKeysDown(int key, int x, int y) {
	// Simply mark the key as pressed
	switch (key) {
		case GLUT_KEY_UP: 	 keyStates[GLUT_KEY_UP] = true; break;
		case GLUT_KEY_DOWN:  keyStates[GLUT_KEY_DOWN] = true; break;
		case GLUT_KEY_LEFT:  keyStates[GLUT_KEY_LEFT] = true; break;
		case GLUT_KEY_RIGHT: keyStates[GLUT_KEY_RIGHT] = true; break;
		// Add other cases if needed
	}
	// We don't call glutPostRedisplay() here anymore
}

// Called when a SPECIAL key is released
void specialKeysUp(int key, int x, int y) {
	// Mark the key as not pressed
	switch (key) {
		case GLUT_KEY_UP: 	 keyStates[GLUT_KEY_UP] = false; break;
		case GLUT_KEY_DOWN:  keyStates[GLUT_KEY_DOWN] = false; break;
		case GLUT_KEY_LEFT:  keyStates[GLUT_KEY_LEFT] = false; break;
		case GLUT_KEY_RIGHT: keyStates[GLUT_KEY_RIGHT] = false; break;
		// Add other cases if needed
	}
}

// Optional: Also add for normal keys if needed
void normalKeysDown(unsigned char key, int x, int y) {
	if (key < 256) {
		keyStates[key] = true;
	}
	if (key == 27) { // ESC key
		exit(0);
	}
	// Example: 'r' to reset tilt
	if (key == 'r' || key == 'R') {
		tiltX = 0.0f;
		tiltZ = 0.0f;
	}
}

void normalKeysUp(unsigned char key, int x, int y) {
	if (key < 256) {
		keyStates[key] = false;
	}
}


// ... (mouseButton, mouseMove remain the same) ...
void mouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isDragging = true;
			lastMouseX = x;
			lastMouseY = y;
		} else {
			isDragging = false;
		}
	}
	else if (button == 3) { // Scroll up
		cameraDistance -= 0.5f;
		if (cameraDistance < 2.0f) cameraDistance = 2.0f;
		glutPostRedisplay();
	} else if (button == 4) { // Scroll down
		cameraDistance += 0.5f;
		if (cameraDistance > 30.0f) cameraDistance = 30.0f;
		glutPostRedisplay();
	}
}

void mouseMove(int x, int y) {
	if (isDragging) {
		int deltaX = x - lastMouseX;
		int deltaY = y - lastMouseY;

		cameraAngleX += deltaX * mouseSensitivity;
		cameraAngleY += deltaY * mouseSensitivity;
		cameraAngleY = clamp(cameraAngleY, 5.0f, 85.0f);

		lastMouseX = x;
		lastMouseY = y;
		glutPostRedisplay();
	}
}

void init() {
	// Initialize key states
	for (int i = 0; i < 256; ++i) {
		keyStates[i] = false;
	}
	// Initialize GLUT special key states (usually > 100) manually if needed for array index safety
	// Although the switch statement handles specific keys, initializing the array helps
	// if you ever check keyStates[some_special_key_code] directly outside the switch.
	// For this specific code using switch, explicit init might not be strictly necessary for
	// arrow keys, but it's good practice if extending it.
	// keyStates[GLUT_KEY_UP] = false; // These are already covered by the loop 0-255
	// keyStates[GLUT_KEY_DOWN] = false;
	// keyStates[GLUT_KEY_LEFT] = false;
	// keyStates[GLUT_KEY_RIGHT] = false;


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	GLfloat light_pos[] = {0.0f, 15.0f, 15.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	GLfloat ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	GLfloat diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	// Make sure window title is simple ASCII
	glutCreateWindow("Marble Tilt Game - Smooth Simultaneous Control");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, timer, 0);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	// --- REGISTER NEW Keyboard Callbacks ---
	glutSpecialFunc(specialKeysDown); // For special keys pressed
	glutSpecialUpFunc(specialKeysUp); 	 // For special keys released
	glutKeyboardFunc(normalKeysDown); // For normal keys pressed
	glutKeyboardUpFunc(normalKeysUp); 	 // For normal keys released

	// Tell GLUT to ignore OS auto-repeat
	glutIgnoreKeyRepeat(1);

	glutMainLoop();
	return 0;
}
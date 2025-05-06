#ifndef INPUT_H
#define INPUT_H

void specialKeysDown(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
void normalKeysDown(unsigned char key, int x, int y);
void normalKeysUp(unsigned char key, int x, int y);
void mouseButton(int button, int state, int x, int y);
void mouseMove(int x, int y);

void initKeyStates(); // Helper to initialize key states

#endif // INPUT_H
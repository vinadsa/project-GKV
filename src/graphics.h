#ifndef GRAPHICS_H
#define GRAPHICS_H

void display();
void reshape(int w, int h);
void timer(int value);
void initGraphics(); // For OpenGL specific initializations
void updateDynamicLighting(); // Update lighting based on marble position

// Forward declaration for initGame function
void initGame();

#endif // GRAPHICS_H
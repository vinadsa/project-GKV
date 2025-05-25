#ifndef TIMER_H
#define TIMER_H

// Call this once at the beginning of your game or when you want to reset the timer
void resetTimer();

// Call this to start or resume the timer
void startTimer();

// Call this to pause the timer
void stopTimer();

// Call this every frame to update the timer's state
void updateTimer();

// Call this in your display function to render the timer on screen
// screenWidth and screenHeight are the current dimensions of your game window
void displayTimer(int screenWidth, int screenHeight);

// Gets the elapsed time as a formatted string (e.g., "MM:SS.mmm")
// Note: The returned string is a pointer to a static buffer, so use it or copy it immediately.
char* getElapsedTimeString();

#endif // TIMER_H
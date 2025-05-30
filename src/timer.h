#ifndef TIMER_H
#define TIMER_H

#include <vector> // Required for std::vector
#include <string> // Required for std::string

// Call this once at the beginning of your game or when you want to reset the timer
void resetTimer();

// Initialize countdown timer with initial time in seconds
void initCountdownTimer(double initialSeconds);

// Add time to the countdown timer
void addTimeToCountdown(double secondsToAdd);

// Check if countdown has reached zero
bool isCountdownExpired();

// Get remaining time in seconds
double getRemainingTime();

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

// New functions for checkpoint timing
void recordCheckpointTime();
const std::vector<double>& getCheckpointTimes(); // Returns a const reference to the vector of checkpoint times
std::vector<std::string> getFormattedCheckpointTimes(); // Returns formatted checkpoint times

#endif // TIMER_H
#include "timer.h"
#include <GL/glut.h>
#include <string>
#include <vector>
#include <iomanip> // For std::setw and std::setfill
#include <sstream> // For std::ostringstream
#include <chrono>  // For time measurement

static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
static std::chrono::duration<double> elapsedSeconds = std::chrono::duration<double>::zero();
static bool isRunning = false;
static char timeString[32]; // Buffer for formatted time string
static std::vector<double> checkpointTimes; // Ensure checkpointTimes is declared here

// Countdown timer variables
static double countdownTime = 60.0; // Default 1 minute
static std::chrono::time_point<std::chrono::high_resolution_clock> countdownStartTime;
static bool countdownRunning = false;

void resetTimer() {
    elapsedSeconds = std::chrono::duration<double>::zero();
    isRunning = false;
    checkpointTimes.clear(); // Clear recorded checkpoint times
    // Reset countdown to initial time
    countdownTime = 60.0; // Reset to 1 minute
    countdownRunning = false;
}

void startTimer() {
    if (!isRunning) {
        startTime = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(elapsedSeconds);
        isRunning = true;
    }
}

void stopTimer() {
    if (isRunning) {
        elapsedSeconds = std::chrono::high_resolution_clock::now() - startTime;
        isRunning = false;
    }
}

void updateTimer() {
    if (isRunning) {
        elapsedSeconds = std::chrono::high_resolution_clock::now() - startTime;
    }
}

// Definition for recordCheckpointTime
void recordCheckpointTime() {
    checkpointTimes.push_back(elapsedSeconds.count());
}

// New function to get formatted checkpoint times
std::vector<std::string> getFormattedCheckpointTimes() {
    std::vector<std::string> formattedTimes;
    // int checkpointNumber = 1; // No longer needed
    for (double timeInSeconds : checkpointTimes) { // Now checkpointTimes should be recognized
        int minutes = static_cast<int>(timeInSeconds) / 60;
        int seconds = static_cast<int>(timeInSeconds) % 60;
        // Corrected: use timeInSeconds for milliseconds calculation, not totalSeconds
        int milliseconds = static_cast<int>((timeInSeconds - static_cast<int>(timeInSeconds)) * 1000);

        std::ostringstream oss;
        // Prepend "Checkpoint X: " to the time  -- REMOVED
        oss << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds << "."
            << std::setw(3) << std::setfill('0') << milliseconds;
        formattedTimes.push_back(oss.str());
        // checkpointNumber++; // No longer needed
    }
    return formattedTimes;
}

char* getElapsedTimeString() {
    // Use countdown timer instead of elapsed time
    double totalSeconds = getRemainingTime();
    int minutes = static_cast<int>(totalSeconds) / 60;
    int seconds = static_cast<int>(totalSeconds) % 60;
    int milliseconds = static_cast<int>((totalSeconds - static_cast<int>(totalSeconds)) * 1000);

    std::ostringstream oss;
    oss << "Time: " << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "."
        << std::setw(3) << std::setfill('0') << milliseconds;
    
    // Copy to static buffer
    snprintf(timeString, sizeof(timeString), "%s", oss.str().c_str());
    return timeString;
}

void displayTimer(int screenWidth, int screenHeight) {
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable lighting and depth test for 2D text rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Set text color (e.g., white)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Set raster position for text rendering
    int mainTimerYPosition = screenHeight - 30;
    glRasterPos2i(10, mainTimerYPosition); // Adjust Y for font size and margin

    char* timeStr = getElapsedTimeString();
    for (char* c = timeStr; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Or another GLUT font
    }    // Checkpoint times display removed - only show countdown timer

    // Restore previous state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void initCountdownTimer(double initialSeconds) {
    countdownTime = initialSeconds;
    countdownStartTime = std::chrono::high_resolution_clock::now();
    countdownRunning = true;
}

void addTimeToCountdown(double secondsToAdd) {
    countdownTime += secondsToAdd;
    // If countdown was expired and we're adding time, restart it
    if (!countdownRunning && countdownTime > 0) {
        countdownStartTime = std::chrono::high_resolution_clock::now();
        countdownRunning = true;
    }
}

bool isCountdownExpired() {
    if (!countdownRunning) return true;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - countdownStartTime);
    double remaining = countdownTime - elapsed.count();
    
    if (remaining <= 0) {
        countdownRunning = false;
        return true;
    }
    return false;
}

double getRemainingTime() {
    if (!countdownRunning) return 0.0;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - countdownStartTime);
    double remaining = countdownTime - elapsed.count();
    
    return remaining > 0 ? remaining : 0.0;
}

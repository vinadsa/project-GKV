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

void resetTimer() {
    elapsedSeconds = std::chrono::duration<double>::zero();
    isRunning = false;
    // Optionally, immediately start the timer upon reset if desired
    // startTimer(); 
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

char* getElapsedTimeString() {
    double totalSeconds = elapsedSeconds.count();
    int minutes = static_cast<int>(totalSeconds) / 60;
    int seconds = static_cast<int>(totalSeconds) % 60;
    int milliseconds = static_cast<int>((totalSeconds - static_cast<int>(totalSeconds)) * 1000);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "."
        << std::setw(3) << std::setfill('0') << milliseconds;
    
    // Copy to static buffer
    // Ensure the buffer is large enough for your format. "MM:SS.mmm" is 9 chars + null terminator.
    // A safer approach would be to return std::string or pass a buffer.
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

    // Position and display the main timer
    // Adjust x and y for desired placement. (0,0) is bottom-left.
    // For top-left, you might use (10, screenHeight - 20) or similar.
    int mainTimerYPosition = screenHeight - 30;
    glRasterPos2i(10, mainTimerYPosition); // Adjust Y for font size and margin

    char* timeStr = getElapsedTimeString();
    for (char* c = timeStr; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Or another GLUT font
    }

    // Display checkpoint times
    std::vector<std::string> checkpointStrings = getFormattedCheckpointTimes();
    int checkpointYPosition = mainTimerYPosition - 20; // Start 20 pixels below the main timer

    if (!checkpointStrings.empty()) {
        // Optional: Display a header for checkpoints
        glRasterPos2i(10, checkpointYPosition);
        std::string checkpointHeader = "Checkpoints:";
        for (char ch : checkpointHeader) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ch);
        }
        checkpointYPosition -= 20; // Move down for the first checkpoint time

        for (const std::string& cptStr : checkpointStrings) {
            glRasterPos2i(10, checkpointYPosition);
            for (char ch : cptStr) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ch);
            }
            checkpointYPosition -= 20; // Move down for the next checkpoint time
        }
    }

    // Restore previous state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

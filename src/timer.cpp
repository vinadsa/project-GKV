#include "timer.h"
#include <GL/glut.h>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <chrono>

static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
static std::chrono::duration<double> elapsedSeconds = std::chrono::duration<double>::zero();
static bool isRunning = false;
static char timeString[32];
static std::vector<double> checkpointTimes;

static double countdownTime = 60.0; 
static std::chrono::time_point<std::chrono::high_resolution_clock> countdownStartTime;
static bool countdownRunning = false;

void resetTimer() {
    elapsedSeconds = std::chrono::duration<double>::zero();
    isRunning = false;
    checkpointTimes.clear();
    countdownTime = 60.0;
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

void recordCheckpointTime() {
    checkpointTimes.push_back(elapsedSeconds.count());
}

std::vector<std::string> getFormattedCheckpointTimes() {
    std::vector<std::string> formattedTimes;
    for (double timeInSeconds : checkpointTimes) {
        int minutes = static_cast<int>(timeInSeconds) / 60;
        int seconds = static_cast<int>(timeInSeconds) % 60;
        int milliseconds = static_cast<int>((timeInSeconds - static_cast<int>(timeInSeconds)) * 1000);

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds << "."
            << std::setw(3) << std::setfill('0') << milliseconds;
        formattedTimes.push_back(oss.str());
    }
    return formattedTimes;
}

char* getElapsedTimeString() {
    double totalSeconds = getRemainingTime();
    int minutes = static_cast<int>(totalSeconds) / 60;
    int seconds = static_cast<int>(totalSeconds) % 60;
    int milliseconds = static_cast<int>((totalSeconds - static_cast<int>(totalSeconds)) * 1000);

    std::ostringstream oss;
    oss << "Time: " << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "."
        << std::setw(3) << std::setfill('0') << milliseconds;
    
    snprintf(timeString, sizeof(timeString), "%s", oss.str().c_str());
    return timeString;
}

void displayTimer(int screenWidth, int screenHeight) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glColor3f(1.0f, 1.0f, 1.0f);

    int mainTimerYPosition = screenHeight - 30;
    glRasterPos2i(10, mainTimerYPosition);

    char* timeStr = getElapsedTimeString();
    for (char* c = timeStr; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }   

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

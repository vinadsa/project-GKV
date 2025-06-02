#ifndef TIMER_H
#define TIMER_H

#include <vector> // Required for std::vector
#include <string> // Required for std::string
void resetTimer();

void initCountdownTimer(double initialSeconds);

void addTimeToCountdown(double secondsToAdd);

bool isCountdownExpired();

double getRemainingTime();

void startTimer();

void stopTimer();

void updateTimer();

void displayTimer(int screenWidth, int screenHeight);

char* getElapsedTimeString();

void recordCheckpointTime();
const std::vector<double>& getCheckpointTimes(); // Returns a const reference to the vector of checkpoint times
std::vector<std::string> getFormattedCheckpointTimes(); // Returns formatted checkpoint times

#endif // TIMER_H
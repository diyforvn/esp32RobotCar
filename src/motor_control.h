#pragma once
#include <Arduino.h>


void initMotors();
void driveMotors(int leftSpeed, int rightSpeed); // -255..255
void stopMotors();
void setMotor(int motorID, int speed);
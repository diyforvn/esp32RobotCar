#pragma once
#include <Arduino.h>


struct RobotConfig {
// PID line-follow
float Kp = 30.0;
float Ki = 0.0;
float Kd = 8.0;


// Speed
int baseSpeed = 120; // 0..255
int maxSpeed = 250;


// Servo scan
int servoLeft = 45;
int servoRight = 135;
int servoCenter = 90;
int servoStep = 10; // degrees per step
int scanIntervalMs = 120; // ms between moves


// Obstacle
int obstacleEnterCm = 20; // threshold to start avoidance
int obstacleClearCm = 30; // threshold considered clear
int obstacleFarCm = 60; // Bắt đầu giảm tốc và quét
int obstacleNearCm = 30; // Gần, cần né hướng
int obstacleCriticalCm = 15; // Quá gần, phải dừng/lùi


// Weights for cost function
float w_dist = 1.0;
float w_angle = 0.8;
float w_line = 0.6;


// Line sensor
int sensorThreshold = 1; // for digital sensors


// Mode
bool autoMode = true;

bool gamePad = true;

bool sensorMode = false;
};


extern RobotConfig config;
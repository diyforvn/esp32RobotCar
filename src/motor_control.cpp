#include "motor_control.h"
#include "debug.h"

// ---------------- Pin mapping ----------------

/*
const int FL_IN1 = 27, FL_IN2 = 25; // Front-left
const int FR_IN1 = 18, FR_IN2 = 5; // Front-right
const int RL_IN1 = 32,  RL_IN2 = 12; // Rear-left
const int RR_IN1 = 4, RR_IN2 = 15; // Rear-right
*/

 int FL_IN1 = 27, FL_IN2 = 25; // Front-left
 int FR_IN1 = 18, FR_IN2 = 5; // Front-right
 int RL_IN1 = 32,  RL_IN2 = 12; // Rear-left
 int RR_IN1 = 4, RR_IN2 = 15; // Rear-right

int buzz = 14;

// ---------------- PWM config ----------------

const int PWM_CH_FL = 1, PWM_CH_FR = 3, PWM_CH_RL = 5, PWM_CH_RR = 7;  // IN1 channels
const int PWM_CH_FL_B = 2, PWM_CH_FR_B = 4, PWM_CH_RL_B = 6, PWM_CH_RR_B = 8; // IN2 channels


const int PWM_FREQ = 20000;
const int PWM_RES = 8;

static int pwmMax = 255;
volatile bool autoMode = false;

// ---------------- Setup ----------------
static void setupPWM() {
    pinMode(buzz, OUTPUT);
    digitalWrite(buzz, LOW);

    // Front-left
    ledcSetup(PWM_CH_FL, PWM_FREQ, PWM_RES);
    ledcAttachPin(FL_IN1, PWM_CH_FL);
    ledcSetup(PWM_CH_FL_B, PWM_FREQ, PWM_RES);
    ledcAttachPin(FL_IN2, PWM_CH_FL_B);

    // Front-right
    ledcSetup(PWM_CH_FR, PWM_FREQ, PWM_RES);
    ledcAttachPin(FR_IN1, PWM_CH_FR);
    ledcSetup(PWM_CH_FR_B, PWM_FREQ, PWM_RES);
    ledcAttachPin(FR_IN2, PWM_CH_FR_B);

    // Rear-left
    ledcSetup(PWM_CH_RL, PWM_FREQ, PWM_RES);
    ledcAttachPin(RL_IN1, PWM_CH_RL);
    ledcSetup(PWM_CH_RL_B, PWM_FREQ, PWM_RES);
    ledcAttachPin(RL_IN2, PWM_CH_RL_B);

    // Rear-right
    ledcSetup(PWM_CH_RR, PWM_FREQ, PWM_RES);
    ledcAttachPin(RR_IN1, PWM_CH_RR);
    ledcSetup(PWM_CH_RR_B, PWM_FREQ, PWM_RES);
    ledcAttachPin(RR_IN2, PWM_CH_RR_B);

    // Stop all PWM
    for (int ch = 1; ch < 9; ch++) ledcWrite(ch, 0);// 1 - 8
}

// ---------------- Core control ----------------
static void setMotorPair(int chA, int chB, int speed) {
    if (speed == 0) {
        ledcWrite(chA, 0);
        ledcWrite(chB, 0);
        return;
    }

    bool forward = (speed > 0);
    int pwm = min(abs(speed), pwmMax);

    if (forward) {
        ledcWrite(chA, pwm);
        ledcWrite(chB, 0);
    } else {
        ledcWrite(chA, 0);
        ledcWrite(chB, pwm);
    }
}

// ---------------- Public functions ----------------
void initMotors() {
    setupPWM();
}

void driveMotors(int leftSpeed, int rightSpeed) {
    // Left side
    setMotorPair(PWM_CH_FL, PWM_CH_FL_B, leftSpeed);
    setMotorPair(PWM_CH_RL, PWM_CH_RL_B, -leftSpeed); // đảo chiều so với front

    // Right side
    setMotorPair(PWM_CH_FR, PWM_CH_FR_B, rightSpeed);
    setMotorPair(PWM_CH_RR, PWM_CH_RR_B, -rightSpeed); // đảo chiều so với front
}

void stopMotors() {
    driveMotors(0, 0);
}

void setMotor(int motorID, int speed) {
    switch (motorID) {
        case 1: setMotorPair(PWM_CH_FL, PWM_CH_FL_B, speed); break; // Front-left
        case 2: setMotorPair(PWM_CH_FR, PWM_CH_FR_B, speed); break; // Front-right
        case 3: setMotorPair(PWM_CH_RL, PWM_CH_RL_B, -speed); break; // Rear-left
        case 4: setMotorPair(PWM_CH_RR, PWM_CH_RR_B, -speed); break; // Rear-right
        default: break;
    }
}

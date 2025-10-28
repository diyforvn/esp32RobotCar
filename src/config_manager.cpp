#include "config_manager.h"
#include <Preferences.h>
#include "debug.h"

RobotConfig config;


void loadConfig() {
Preferences prefs;
prefs.begin("robot", true);
config.Kp = prefs.getFloat("Kp", config.Kp);
config.Ki = prefs.getFloat("Ki", config.Ki);
config.Kd = prefs.getFloat("Kd", config.Kd);
config.baseSpeed = prefs.getInt("baseSpeed", config.baseSpeed);
config.maxSpeed = prefs.getInt("maxSpeed", config.maxSpeed);
config.servoLeft = prefs.getInt("servoLeft", config.servoLeft);
config.servoRight = prefs.getInt("servoRight", config.servoRight);
config.servoCenter = prefs.getInt("servoCenter", config.servoCenter);
config.servoStep = prefs.getInt("servoStep", config.servoStep);
config.scanIntervalMs = prefs.getInt("scanIntervalMs", config.scanIntervalMs);
config.obstacleEnterCm = prefs.getInt("obstacleEnterCm", config.obstacleEnterCm);
config.obstacleClearCm = prefs.getInt("obstacleClearCm", config.obstacleClearCm);
config.obstacleFarCm = prefs.getInt("obstacleFarCm", config.obstacleFarCm);
config.obstacleNearCm = prefs.getInt("obstacleNearCm", config.obstacleNearCm);
config.obstacleCriticalCm = prefs.getInt("obstacleCriticalCm", config.obstacleCriticalCm);

config.w_dist = prefs.getFloat("w_dist", config.w_dist);
config.w_angle = prefs.getFloat("w_angle", config.w_angle);
config.w_line = prefs.getFloat("w_line", config.w_line);
config.sensorThreshold = prefs.getInt("sensorThreshold", config.sensorThreshold);
config.autoMode = prefs.getBool("autoMode", config.autoMode);
config.gamePad = prefs.getBool("gamePad", config.gamePad);
config.sensorMode = prefs.getBool("sensorMode", config.sensorMode);

prefs.end();

    Serial.println(F("<<<< Robot Config >>>>"));
  Serial.print(F("Kp: ")); Serial.println(config.Kp, 6);
  Serial.print(F("Ki: ")); Serial.println(config.Ki, 6);
  Serial.print(F("Kd: ")); Serial.println(config.Kd, 6);
  Serial.print(F("baseSpeed: ")); Serial.println(config.baseSpeed);
  Serial.print(F("maxSpeed: ")); Serial.println(config.maxSpeed);
  Serial.print(F("servoLeft: ")); Serial.println(config.servoLeft);
  Serial.print(F("servoRight: ")); Serial.println(config.servoRight);
  Serial.print(F("servoCenter: ")); Serial.println(config.servoCenter);
  Serial.print(F("servoStep: ")); Serial.println(config.servoStep);
  Serial.print(F("scanIntervalMs: ")); Serial.println(config.scanIntervalMs);
  Serial.print(F("obstacleEnterCm: ")); Serial.println(config.obstacleEnterCm);
  Serial.print(F("obstacleClearCm: ")); Serial.println(config.obstacleClearCm);

  Serial.print(F("obstacleFarCm: ")); Serial.println(config.obstacleFarCm);
  Serial.print(F("obstacleNearCm: ")); Serial.println(config.obstacleNearCm);
  Serial.print(F("obstacleCriticalCm: ")); Serial.println(config.obstacleCriticalCm);

  Serial.print(F("w_dist: ")); Serial.println(config.w_dist, 6);
  Serial.print(F("w_angle: ")); Serial.println(config.w_angle, 6);
  Serial.print(F("w_line: ")); Serial.println(config.w_line, 6);
  Serial.print(F("sensorThreshold: ")); Serial.println(config.sensorThreshold);
  Serial.print(F("autoMode: "));
  Serial.println(config.autoMode ? "true" : "false");
  Serial.print(F("gamePad: "));
  Serial.println(config.gamePad ? "true" : "false");
  Serial.print(F("runMode: "));
  Serial.println(config.sensorMode ? "lineSensor" : "ultrasonic");

  Serial.println(F("----------------------"));
}


void saveConfig() {
Preferences prefs;
prefs.begin("robot", false);
prefs.putFloat("Kp", config.Kp);
prefs.putFloat("Ki", config.Ki);
prefs.putFloat("Kd", config.Kd);
prefs.putInt("baseSpeed", config.baseSpeed);
prefs.putInt("maxSpeed", config.maxSpeed);
prefs.putInt("servoLeft", config.servoLeft);
prefs.putInt("servoRight", config.servoRight);
prefs.putInt("servoCenter", config.servoCenter);
prefs.putInt("servoStep", config.servoStep);
prefs.putInt("scanIntervalMs", config.scanIntervalMs);
prefs.putInt("obstacleEnterCm", config.obstacleEnterCm);
prefs.putInt("obstacleClearCm", config.obstacleClearCm);

prefs.putInt("obstacleFarCm", config.obstacleFarCm);
prefs.putInt("obstacleNearCm", config.obstacleNearCm);
prefs.putInt("obstacleCriticalCm", config.obstacleCriticalCm);

prefs.putFloat("w_dist", config.w_dist);
prefs.putFloat("w_angle", config.w_angle);
prefs.putFloat("w_line", config.w_line);
prefs.putInt("sensorThreshold", config.sensorThreshold);
prefs.putBool("autoMode", config.autoMode);
prefs.putBool("gamePad", config.gamePad);
prefs.putBool("sensorMode", config.sensorMode);
prefs.end();
Serial.println(F("-------- Save config OK --------------"));

   /* Serial.println(F(">>>> Saved Robot Config <<<<"));
  Serial.print(F("Kp: ")); Serial.println(config.Kp, 6);
  Serial.print(F("Ki: ")); Serial.println(config.Ki, 6);
  Serial.print(F("Kd: ")); Serial.println(config.Kd, 6);
  Serial.print(F("baseSpeed: ")); Serial.println(config.baseSpeed);
  Serial.print(F("maxSpeed: ")); Serial.println(config.maxSpeed);
  Serial.print(F("servoLeft: ")); Serial.println(config.servoLeft);
  Serial.print(F("servoRight: ")); Serial.println(config.servoRight);
  Serial.print(F("servoCenter: ")); Serial.println(config.servoCenter);
  Serial.print(F("servoStep: ")); Serial.println(config.servoStep);
  Serial.print(F("scanIntervalMs: ")); Serial.println(config.scanIntervalMs);
  Serial.print(F("obstacleEnterCm: ")); Serial.println(config.obstacleEnterCm);
  Serial.print(F("obstacleClearCm: ")); Serial.println(config.obstacleClearCm);
  Serial.print(F("w_dist: ")); Serial.println(config.w_dist, 6);
  Serial.print(F("w_angle: ")); Serial.println(config.w_angle, 6);
  Serial.print(F("w_line: ")); Serial.println(config.w_line, 6);
  Serial.print(F("sensorThreshold: ")); Serial.println(config.sensorThreshold);
  Serial.print(F("autoMode: "));
  Serial.println(config.autoMode ? "true" : "false");
  Serial.println(F("----------------------"));*/
}
#pragma once
#include <Arduino.h>

// ===== Khai báo extern handle của tất cả các task =====
extern TaskHandle_t hTaskPs4;
extern TaskHandle_t hTaskBLE;
extern TaskHandle_t hTaskUltrasonic;
extern TaskHandle_t hTaskLine;
//extern TaskHandle_t hTaskWeb;
extern TaskHandle_t hTaskServo;

// ===== Hàm khởi tạo chung =====
void createAllTasks();
void deleteAllTasks();

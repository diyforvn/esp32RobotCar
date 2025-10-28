#include "config_manager.h"
#include "task_manager.h"
#include "motor_control.h"
#include "line_sensor.h"
#include "ultrasonic.h"
#include "servo_scan.h"
#include "web_config.h"
#include "ble_ps4.h"
#include "ble_manual.h"
#include "debug.h"

// ===== Định nghĩa thật các biến handle =====
TaskHandle_t hTaskPs4 = NULL;
TaskHandle_t hTaskBLE = NULL;
TaskHandle_t hTaskUltrasonic = NULL;
TaskHandle_t hTaskLine = NULL;
//TaskHandle_t hTaskWeb = NULL;
TaskHandle_t hTaskServo = NULL;

// ===== Tạo tất cả các task cần thiết =====
void createAllTasks() {
    DEBUG_PRINTLN("===> Creating all tasks...");

    if (config.gamePad) xTaskCreatePinnedToCore(Task_PS4, "PS4", 4096, NULL, 1, &hTaskPs4, 1);
    else xTaskCreatePinnedToCore(Task_BLE, "BLE", 4096, NULL, 1, &hTaskBLE, 1);
    //xTaskCreatePinnedToCore(ultrasonicTask, "Ultrasonic", 2048, NULL, 1, &hTaskUltrasonic, 1);
    //xTaskCreatePinnedToCore(Task_LineReader, "LineReader", 4096, NULL, 3, &hTaskLine, 1);
    //xTaskCreatePinnedToCore(webTask, "Web", 4096, NULL, 1, &hTaskWeb, 1);
    //xTaskCreatePinnedToCore(Task_ServoScan, "ServoScan", 8192, NULL, 2, &hTaskServo, 1); 
}

// ===== Hàm xóa tất cả các task =====
void deleteAllTasks() {
    DEBUG_PRINTLN("===> Deleting all tasks...");
    if (hTaskPs4)      { vTaskDelete(hTaskPs4);      hTaskPs4 = NULL; }
    if (hTaskBLE)      { vTaskDelete(hTaskBLE);      hTaskBLE = NULL; }
    if (hTaskUltrasonic){ vTaskDelete(hTaskUltrasonic);hTaskUltrasonic = NULL; }
    if (hTaskLine)     { vTaskDelete(hTaskLine);     hTaskLine = NULL; }
    //if (hTaskWeb)      { vTaskDelete(hTaskWeb);      hTaskWeb = NULL; }
}

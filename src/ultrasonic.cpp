#include "ultrasonic.h"
#include <Arduino.h>
#include "debug.h"

extern TaskHandle_t hTaskUltrasonic;
const int TRIG_PIN = 23;
const int ECHO_PIN = 19;

static int distanceCM = 0;


// ===== Khởi tạo chân =====
void initUltrasonic() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    xTaskCreatePinnedToCore(ultrasonicTask, "Ultrasonic", 2048, NULL, 1, &hTaskUltrasonic, 1);
}

// ===== Hàm đo một lần (dành cho nội bộ task) =====
static int pingOnce() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
    if (duration == 0) return 9999;
    return duration / 58;
}

// ===== Task đo liên tục =====
void ultrasonicTask(void *param) {
    for (;;) {
        int cm = pingOnce();
        distanceCM = cm;
        DEBUG_PRINTLN("Ultrasonic: " + String(cm) + " cm");
        vTaskDelay(70 / portTICK_PERIOD_MS); // đo mỗi 70ms
    }
}

// ===== Lấy giá trị đo hiện tại =====
int getDistanceCM() {
    return distanceCM;
}

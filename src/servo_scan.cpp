#include "servo_scan.h"
#include "ultrasonic.h"
#include "motor_control.h"
#include "line_sensor.h"
#include "config_struct.h"
#include <ESP32Servo.h>
#include <Arduino.h>
#include "debug.h"


extern RobotConfig config;
static Servo scanServo;
static int currentAngle;
static int step;
static unsigned long lastMoveMs = 0;

const int SERVO_PIN = 26;
extern TaskHandle_t hTaskServo;

void initServoScan() {
    scanServo.attach(SERVO_PIN);
    currentAngle = config.servoCenter;
    step = config.servoStep;
    scanServo.write(currentAngle);
    xTaskCreatePinnedToCore(Task_ServoScan, "ServoScan", 8192, NULL, 2, &hTaskServo, 1); 
}


struct ScanResult {
    int angle;    // Góc tốt nhất tìm được
    int distance; // Khoảng cách xa nhất tại góc đó
};
// --- 2.  `evaluateScanWindow` ---
/**
 * Quét một vùng góc và tìm góc có khoảng cách xa nhất.
 * @param center Góc trung tâm để quét
 * @param radius Bán kính quét (ví dụ: center=90, radius=90 sẽ quét từ 0-180)
 * @return Struct ScanResult chứa góc tốt nhất và khoảng cách tốt nhất
 */
static ScanResult evaluateScanWindow(int center, int radius) {
    ScanResult best = {center, -1}; // {angle, distance}
    int step = max(4, config.servoStep);

    // Xác định giới hạn quét, đảm bảo không vượt quá biên độ servo
    int start = max(config.servoLeft, center - radius);
    int end   = min(config.servoRight, center + radius);

    for (int a = start; a <= end; a += step) {
        scanServo.write(a);
        // Delay 30ms đủ cho servo di chuyển VÀ cảm biến ping
        vTaskDelay(pdMS_TO_TICKS(30));

        int d = getDistanceCM();
        if (d <= 2) continue; // Bỏ qua nhiễu/đọc lỗi

        if (d > best.distance) {
            best.distance = d;
            best.angle = a;
        }
    }

    // Quay servo về góc tốt nhất đã tìm thấy (hoặc có thể về giữa)
    // scanServo.write(best.angle);
    return best;
}

// --- 3. TASK CHÍNH ---
void Task_ServoScan(void *pv)
{
    (void)pv;

    enum State {
        DRIVE_FORWARD,        // Đi thẳng và quét phía trước
        FIND_CLEAR_DIRECTION, // Dừng và quét toàn diện tìm lối thoát
        ROTATE_TO_CLEAR       // Xoay robot về hướng đã chọn
    };
    State state = DRIVE_FORWARD;

    int direction = 1; // -1 = xoay trái, 1 = xoay phải
    unsigned long stateStart = millis();
    unsigned long lastCheck = 0;
    unsigned long lastSweep = 0;
    unsigned long now = 0;

    int d = 0;
    ScanResult escapeRoute;

    // Biến cho việc quét khi đi thẳng
    static int sweepAngle = config.servoCenter;
    static int sweepDir = 1; // 1 = quét sang phải, -1 = quét sang trái

    for (;;) {
        if (!config.autoMode) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        now = millis();

        // Đọc cảm biến khoảng cách mỗi 80ms
        // Cảm biến sẽ đọc ở `sweepAngle` hiện tại
        if (now - lastCheck > 80) {
            d = getDistanceCM();
            lastCheck = now;
        }

        switch (state)
        {
        // =========================================================
        case DRIVE_FORWARD:
        {
            // --- Quét servo liên tục +/- 45 độ khi đi ---
            if (now - lastSweep > 50) // Tốc độ quét
            {
                // Giả định góc nhỏ là bên phải, góc lớn là bên trái
                // (Dựa trên code gốc: center-90 là phải)
                int sweepMin = config.servoCenter - 45;
                int sweepMax = config.servoCenter + 45;

                sweepAngle += sweepDir * 5; // Bước quét là 5 độ

                if (sweepAngle >= sweepMax) {
                    sweepAngle = sweepMax;
                    sweepDir = -1; // Đổi hướng quét (sang phải)
                } else if (sweepAngle <= sweepMin) {
                    sweepAngle = sweepMin;
                    sweepDir = 1; // Đổi hướng quét (sang trái)
                }

                scanServo.write(sweepAngle);
                lastSweep = now;
            }

            // Logic đi thẳng dựa trên khoảng cách `d` (đã quét)
            if (d > config.obstacleFarCm) {
                // Xa vật cản → đi thẳng
                driveMotors(-config.baseSpeed, -config.baseSpeed);
            }
            else if (d > config.obstacleNearCm) {
                // Gần vật cản → giảm tốc
                int spd = map(d, config.obstacleNearCm, config.obstacleFarCm,
                                150, config.baseSpeed);
                spd = constrain(spd, 150, config.baseSpeed);
                driveMotors(-spd, -spd);
            }
            else {
                // Quá gần (phát hiện bởi chùm quét) → Dừng và tìm hướng
                stopMotors();
                state = FIND_CLEAR_DIRECTION;
                stateStart = now;
            }
            break;
        }

        // =========================================================
        case FIND_CLEAR_DIRECTION:
        {
            // ---  Quét 180 độ để tìm lối thoát tốt nhất ---
            stopMotors();
            
            // Quét toàn bộ 180 độ (hoặc phạm vi servo cho phép)
            escapeRoute = evaluateScanWindow(config.servoCenter, 90);

            // Quay servo về giữa để chuẩn bị xoay robot
            scanServo.write(config.servoCenter);
            sweepAngle = config.servoCenter; // Reset góc quét

            // Quyết định hướng xoay
            if (escapeRoute.distance < config.obstacleFarCm) {
                // Bị kẹt! Lối thoát tốt nhất vẫn quá gần.
                // Quyết định xoay 180 độ (mặc định xoay phải)
                direction = 1;
            } else {
                // Tìm thấy đường thoáng. Quyết định hướng xoay.
                // Dựa trên code gốc: góc nhỏ (center-90) là PHẢI
                //                      góc lớn (center+90) là TRÁI
                if (escapeRoute.angle < config.servoCenter - 10) { // -10 để tạo deadzone
                    direction = 1; // Xoay phải (về phía góc nhỏ)
                }
                else if (escapeRoute.angle > config.servoCenter + 10) { // +10 để tạo deadzone
                    direction = -1; // Xoay trái (về phía góc lớn)
                }
                else {
                    // Lối thoát tốt nhất là gần phía trước
                    // Robot không cần xoay nhiều, nhưng vì đã dừng
                    // nên cứ đi thẳng. (Hoặc xoay nhẹ)
                    // Chuyển về DRIVE_FORWARD, nó sẽ tự đi
                    state = DRIVE_FORWARD;
                    stateStart = now;
                    break; // Bỏ qua chuyển sang ROTATE
                }
            }

            // Chuyển sang trạng thái xoay
            state = ROTATE_TO_CLEAR;
            stateStart = now;
            break;
        }

        // =========================================================
        case ROTATE_TO_CLEAR:
        {
            scanServo.write(config.servoCenter); // Luôn nhìn thẳng khi xoay
            int distNow = getDistanceCM();

            if (distNow > config.obstacleFarCm + 5) {
                // Phía trước đã thoáng → dừng xoay và đi tiếp
                stopMotors();
                state = DRIVE_FORWARD;
                stateStart = now;
                break;
            }

            // Tiếp tục xoay theo hướng đã chọn
            if (direction == 1)
                driveMotors(-180, 180); // xoay phải
            else // direction == -1
                driveMotors(180, -180); // xoay trái

            // --- Nếu xoay lâu quá (5s) mà vẫn kẹt -> quét lại ---
            if (now - stateStart > 5000) {
                // Lựa chọn có thể đã sai, hoặc tình hình thay đổi
                stopMotors();
                state = FIND_CLEAR_DIRECTION; // Quét lại từ đầu
                stateStart = now;
            }
            break;
        }
        } // end switch

        vTaskDelay(pdMS_TO_TICKS(20)); // non-blocking loop
    }
}



#include <Arduino.h>
#include "config_manager.h"
#include "motor_control.h"
#include "line_sensor.h"
#include "ultrasonic.h"
#include "servo_scan.h"
#include "web_config.h"
#include "ble_ps4.h"
#include "ble_manual.h"
#include "task_manager.h"
#include "debug.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"


//#define testMode



// PID internals
static float integralTerm = 0;
static float lastError = 0;
static unsigned long lastPidTime = 0;

#define BTN_PIN 0   // chân nút 
#define LED_STATUS 2
#define HOLD_TIME 3000  // giữ 3 giây

bool wifiMode = false;
bool btnPressed = false;
unsigned long pressStart = 0;
extern int buzz;

extern int FL_IN1, FL_IN2; // Front-left
extern int FR_IN1, FR_IN2; // Front-right
extern int RL_IN1,  RL_IN2; // Rear-left
extern int RR_IN1, RR_IN2; // Rear-right

unsigned long prevMillis = 0;
const unsigned long interval = 100;  // 100ms
const float MAX_ERR = 2.0;
static int lostCount = 0;

bool ledState = false;
// FreeRTOS task handles
TaskHandle_t hTaskPID;

// Global line error used by PID task
float g_lineError = 999.0;



void Task_PID(void *pv) {
  (void)pv;
  static unsigned long lastPidTime = 0;
  static float integralTerm = 0.0f;
  static float lastErrorLocal = 0.0f;
  static float lastDerivative = 0.0f;

  const float DERIV_LPF_ALPHA = 0.4f;
  const float INTEGRAL_LIMIT = 500.0f;

  for (;;) {
    if (!config.autoMode) { vTaskDelay(pdMS_TO_TICKS(50)); continue; }

    float err = g_lineError;

    unsigned long now = millis();
    float dt = (lastPidTime == 0) ? 0.02f : (now - lastPidTime) / 1000.0f;
    if (dt <= 0) dt = 0.02f;
    lastPidTime = now;

    // PID terms
    integralTerm += err * dt;
    integralTerm = constrain(integralTerm, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

    float rawDerivative = (err - lastErrorLocal) / dt;
    float derivative = DERIV_LPF_ALPHA * rawDerivative + (1.0f - DERIV_LPF_ALPHA) * lastDerivative;
    derivative = constrain(derivative, -1000, 1000);
    lastDerivative = derivative;
    lastErrorLocal = err;

  
      float absErr = fabs(err);
      float speedScale = 1.0f;
      float correction_multiplier = 1.0f;

      // Khi sai số càng lớn, giảm tốc càng nhiều và bẻ lái càng gắt
      if (absErr > 2.0f) { // Gần như mất line
          speedScale = 0.4f; // Giảm tốc thật mạnh
          correction_multiplier = 1.5f; // Bẻ lái gắt hơn
      } else if (absErr > 1.5f) {
          speedScale = 0.55f;
          correction_multiplier = 1.2f;
      } else if (absErr > 0.9f) {
          speedScale = 0.75f;
      }

      int dynamicBase = config.baseSpeed * speedScale;
      dynamicBase = constrain(dynamicBase, 180, config.maxSpeed);
     
      float corr = config.Kp * err + config.Ki * integralTerm + config.Kd * derivative;
      corr *= correction_multiplier;


    const float MAX_CORR = config.maxSpeed;
    corr = constrain(corr, -MAX_CORR, MAX_CORR);

    int left = dynamicBase + (int)round(corr);
    int right = dynamicBase - (int)round(corr);

    left = constrain(left, -config.maxSpeed, config.maxSpeed);
    right = constrain(right, -config.maxSpeed, config.maxSpeed);

    driveMotors(-left, -right);

    // Debug
    static unsigned long lastPrint = 0;
    if (now - lastPrint >= 200) {
      Serial.printf("ERR: %.2f  CORR: %.2f  L:%d R:%d  I:%.2f D:%.2f\n",
                    err, corr, left, right, integralTerm, derivative);
      lastPrint = now;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}



String inputString = "";

void serialTestMotors() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    inputString.trim(); // xóa khoảng trắng đầu/cuối

    if (inputString.length() == 0) return;

    // Cú pháp: M<id> <dir> <speed>
    // Ví dụ: "M1 R 200" hoặc "M3 L 100" hoặc "M2 S"
    char m;
    int id, speed = 0;
    char dir;

    int matched = sscanf(inputString.c_str(), "%c%d %c %d", &m, &id, &dir, &speed);
    if (matched >= 3 && m == 'M') {
      if (dir == 'S') {
        setMotor(id, 0);
        Serial.printf("Motor %d STOP\n", id);
      } else {
        if (dir == 'L' || dir == 'R') {
          int realSpeed = (dir == 'L') ? -abs(speed) : abs(speed);
          setMotor(id, realSpeed);
          Serial.printf("Motor %d: %s speed=%d\n", id, (dir == 'L') ? "LEFT" : "RIGHT", abs(speed));
        } else {
          Serial.println("Sai cú pháp! Dùng: M<id> L/R/S [speed]");
        }
      }
    } else {
      Serial.println("Sai định dạng lệnh! Ví dụ: M1 R 200 hoặc M1 S");
    }
  }
}

void disableBluetoothFully() {
  Serial.println("[BT] Stopping Bluetooth stack...");
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  Serial.println("[BT] Bluetooth fully disabled.");
}

void blinkLed() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis >= interval) {
    prevMillis = currentMillis;
    ledState = !ledState;  // Đảo trạng thái
    digitalWrite(LED_STATUS, ledState);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(FL_IN1, OUTPUT);
  pinMode(FL_IN2, OUTPUT);
  pinMode(FR_IN1, OUTPUT);
  pinMode(FR_IN2, OUTPUT);
  pinMode(RL_IN1, OUTPUT);
  pinMode(RL_IN2, OUTPUT);
  pinMode(RR_IN1, OUTPUT);
  pinMode(RR_IN2, OUTPUT);
  digitalWrite(LED_STATUS, LOW);
  digitalWrite(FL_IN1,LOW);
  digitalWrite(FL_IN2,LOW);
  digitalWrite(FR_IN1,LOW);
  digitalWrite(FR_IN2,LOW);
  digitalWrite(RL_IN1,LOW);
  digitalWrite(RL_IN2,LOW);
  digitalWrite(RR_IN1,LOW);
  digitalWrite(RR_IN2,LOW);
  

  loadConfig();
  initMotors();
  if (config.sensorMode) // line sensor
  {
    Serial.println("#### Line sensor used ###");
    initLineSensors();
    xTaskCreatePinnedToCore(Task_PID, "PID", 4096, NULL, 3, &hTaskPID, 1);
  }
  else
  {
    Serial.println("#### Ultrasonic sensor used ###");
    initUltrasonic();
    initServoScan();
  }
  
  #ifndef testMode
    if (config.gamePad) initPS4(); 
    else initBLE();
  #else
    Serial.println("=== Motor Test Mode ===");
    Serial.println("Cú pháp: M<id> <L/R/S> <speed>");
    Serial.println("VD: M1 R 200  → Motor 1 quay phải tốc độ 200");
    Serial.println("    M3 L 120  → Motor 3 quay trái tốc độ 120");
    Serial.println("    M2 S      → Motor 2 dừng");
  #endif
  
   createAllTasks(); // tạo tất cả task nền

   for (int i=0;i<10;i++)
   {
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
    delay(100);
   }

  DEBUG_PRINTLN("System ready");
}

void loop() {
   #ifdef testMode
    serialTestMotors();
  #endif
  if (wifiMode) blinkLed();

  if (digitalRead(BTN_PIN) == LOW) {
    if (!btnPressed) {
      pressStart = millis();
      btnPressed = true;
    } else if (millis() - pressStart > HOLD_TIME && !wifiMode) {
      Serial.println("\n[MODE SWITCH] → WiFi Config mode");      

      if (config.gamePad)
      {
          vTaskDelete(hTaskPs4);
          hTaskPs4 = NULL;
      }
      else
      {
          vTaskDelete(hTaskBLE);
          hTaskBLE = NULL;
      }
      delay(100);
      // Tắt Bluetooth
      Serial.println("Stopping Bluetooth...");
      disableBluetoothFully();
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      delay(100);
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      
      initWebConfig();
      wifiMode = true;
    }
  } else {
    btnPressed = false;
  }


  delay(5);
}


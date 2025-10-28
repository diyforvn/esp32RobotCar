#include <Arduino.h>
#include <PS4Controller.h>
#include "motor_control.h"
#include "config_struct.h"
#include "config_manager.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"
#include "debug.h"

extern RobotConfig config;
extern volatile bool autoMode;

unsigned long lastTimeStamp = 0;
int connectState = 0;

// --- Khai báo biến toàn cục ---
bool lastCircleState   = false;
bool lastCrossState    = false;
bool lastTriangleState = false;
bool lastSquareState   = false;
bool lastUpState       = false;
bool lastDownState     = false;
bool lastLeftState     = false;
bool lastRightState    = false;
bool lastTouchPadState = false;

unsigned long lastSpeedAdjust = 0;
const unsigned long adjustInterval = 100; // 100ms mỗi lần tăng/giảm

bool buzzerOn = false;
unsigned long buzzerStart = 0;

extern int buzz;


bool isPressedOnce(bool current, bool &last) {
  bool result = current && !last;
  last = current;
  return result;
}


// ====== Hàm xóa thiết bị đã pair để tránh lỗi kết nối ======
void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

// ====== In địa chỉ MAC Bluetooth ======
void printDeviceAddress() {
  const uint8_t* point = esp_bt_dev_get_address();
  for (int i = 0; i < 6; i++) {
    char str[3];
    sprintf(str, "%02x", (int)point[i]);
    Serial.print(str);
    if (i < 5) Serial.print(":");
  }
}

// ====== Sự kiện khi tay cầm kết nối ======
void onConnect() {
  connectState = 1;
  Serial.println("🎮 Tay cầm PS4 đã kết nối!");
   digitalWrite(buzz, HIGH);
   delay(100);
   digitalWrite(buzz, LOW);
}

// ====== Sự kiện khi tay cầm ngắt kết nối ======
void onDisConnect() {
  connectState = 0;
  stopMotors();
  Serial.println("⚠️ Tay cầm PS4 ngắt kết nối!");
}

void buzzSound(int number)
{
  digitalWrite(buzz, LOW);
  for (int i=0;i<number*2;i++)
  {
    digitalWrite(buzz, !digitalRead(buzz)); 
    delay(100);       
  }
  digitalWrite(buzz, LOW);
}

// ====== Callback đọc nút & joystick ======
void notify() {
  // Đọc joystick
  int LY = -PS4.LStickY(); // -128 ~ +127
  int LX = PS4.LStickX(); // -128 ~ +127

  int baseSpeed = config.baseSpeed;

  int LY_scaled = LY * baseSpeed / 127;
  int LX_scaled = LX * baseSpeed / 127;

  int leftSpeed  = constrain(LY_scaled - LX_scaled,  -config.maxSpeed, config.maxSpeed);
  int rightSpeed = constrain(LY_scaled + LX_scaled, -config.maxSpeed, config.maxSpeed);

  if (isPressedOnce(PS4.Circle(), lastCircleState)) 
  {
     saveConfig();
    DEBUG_PRINTLN("Save config");
    buzzSound(1);
  }

  if (isPressedOnce(PS4.Touchpad(), lastTouchPadState)) 
  {
     if (config.autoMode == false)
     {
      config.autoMode = true;
      DEBUG_PRINTLN("Auto mode");
      buzzSound(2);
     }
     else
     {
        config.autoMode = false;
        DEBUG_PRINTLN("Manual Auto");
        buzzSound(2);
     }
     
  }

    if (config.autoMode == false)
    {
      if (PS4.Right()) {
        leftSpeed = -baseSpeed;
        rightSpeed = baseSpeed;
        DEBUG_PRINTLN("[PS4] Right");
      }

      if (PS4.Left()) {
        leftSpeed = baseSpeed;
        rightSpeed = -baseSpeed;
        DEBUG_PRINTLN("[PS4] Left");
      }

      if (PS4.Up()) {
        leftSpeed = -baseSpeed;
        rightSpeed = -baseSpeed;
        DEBUG_PRINTLN("[PS4] Up");
      }

      if (PS4.Down()) {
        leftSpeed = baseSpeed;
        rightSpeed = baseSpeed;
        DEBUG_PRINTLN("[PS4] Down");
      }
    
      if (isPressedOnce(PS4.Cross(), lastCrossState)) 
      {
        stopMotors();
        leftSpeed=0;
        rightSpeed=0;
        DEBUG_PRINTLN("⛔ Stop");
        buzzSound(1);
      }
      // Nhấn nút để bật còi
      if (isPressedOnce(PS4.Square(), lastSquareState)) {
        DEBUG_PRINTLN("Buzz ---");
        digitalWrite(buzz, HIGH);
        buzzerOn = true;
        buzzerStart = millis();
      }

      // Tự tắt còi sau 100ms mà không chặn các nút khác
      if (buzzerOn && millis() - buzzerStart > 100) {
        digitalWrite(buzz, LOW);
        buzzerOn = false;
      }

      if (millis() - lastSpeedAdjust > adjustInterval) {
        if (PS4.L2Value() > 50) {
          config.baseSpeed = max(0, config.baseSpeed - 5);
          lastSpeedAdjust = millis();
        }
        if (PS4.R2Value() > 50) {
          config.baseSpeed = min(255, config.baseSpeed + 5);
          lastSpeedAdjust = millis();
        }
      }

      // In giá trị joystick mỗi 50ms để debug
      if(debugOn)
      {
        if (millis() - lastTimeStamp > 50) {
          Serial.printf("LX:%4d  LY:%4d  → L:%4d  R:%4d -> Base:%3d\n", LX, LY, leftSpeed, rightSpeed, config.baseSpeed);
          lastTimeStamp = millis();
        }
      }
      driveMotors(leftSpeed, rightSpeed);
    }

}

// ====== Hàm khởi tạo PS4 ======
void initPS4() {
  Serial.println("🎮 Khởi tạo PS4 controller...");

  
  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin();
  removePairedDevices();  // Giúp tránh lỗi reconnect
  Serial.print("Bluetooth MAC: ");
  printDeviceAddress();
  Serial.println("");
  
  Serial.println("🎮 Đang chờ tay cầm PS4 kết nối...");
}

// ====== Task chính điều khiển bằng tay ======
void Task_PS4(void *pv) {
  (void)pv;

  for (;;) {
    if (connectState == 0 || !PS4.isConnected()) {
      stopMotors();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

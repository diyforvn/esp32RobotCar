#include <NimBLEDevice.h>
#include "motor_control.h"
#include "config_struct.h"
#include "config_manager.h"

extern RobotConfig config;
extern volatile bool autoMode;

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pTxCharacteristic = nullptr;
NimBLECharacteristic* pRxCharacteristic = nullptr;
bool deviceConnected = false;
std::string rxBuffer;

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

unsigned long lastCmdTime = 0;
const unsigned long TIMEOUT_MS = 1000; // 1 giây không nhận lệnh thì dừng

SemaphoreHandle_t bleMutex;  // bảo vệ rxBuffer

extern int buzz;

// Callback khi thiết bị kết nối / ngắt
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    deviceConnected = true;
    Serial.println("✅ Thiết bị BLE đã kết nối!");
    digitalWrite(buzz, HIGH);
   delay(100);
   digitalWrite(buzz, LOW);
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    deviceConnected = false;
    Serial.println("⚠️ Thiết bị BLE ngắt kết nối!");
    NimBLEDevice::startAdvertising();  // Quảng bá lại để thiết bị khác kết nối
    stopMotors();
  }
};


// ====== Characteristic Callback ======
class RxCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    std::string value = pCharacteristic->getValue();
    if (!value.empty()) {
      if (xSemaphoreTake(bleMutex, portMAX_DELAY)) {
        rxBuffer += value;
        xSemaphoreGive(bleMutex);
      }    
      Serial.print("📩 Nhận từ App: ");
      Serial.println(value.c_str());
    }
  }
};

// ====== Hàm in địa chỉ MAC BLE ======
void printDeviceAddress_NIM() {
  NimBLEAddress addr = NimBLEDevice::getAddress();
  Serial.print("BLE MAC: ");
  Serial.println(addr.toString().c_str());
}

void initBLE() {
  bleMutex = xSemaphoreCreateMutex();

  NimBLEDevice::init("ESP32_Robot");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        NIMBLE_PROPERTY::NOTIFY
                      );

  pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        NIMBLE_PROPERTY::WRITE
                      );
  pRxCharacteristic->setCallbacks(new RxCallback());

  pService->start();
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

   Serial.println("Bluetooth OK!");
  printDeviceAddress_NIM();

  Serial.println("✅ BLE UART đã khởi tạo, sẵn sàng kết nối!");  
}

void sendBLE(const std::string& msg) {
  if (!deviceConnected || !pTxCharacteristic) return;

  size_t start = 0;
  while (start < msg.size()) {
    std::string chunk = msg.substr(start, 18); // <= 20 byte
    pTxCharacteristic->setValue(chunk);
    pTxCharacteristic->notify();
    start += 18;
    delay(20);
  }
}

// ====== Task xử lý BLE ======
void Task_BLE(void *pv) {
  (void)pv;
  for (;;) { 
    if (xSemaphoreTake(bleMutex, portMAX_DELAY)) {
      if (!rxBuffer.empty()) {
        // Sao chép toàn bộ nội dung nhận được
        std::string cmd = rxBuffer;
        rxBuffer.clear();
        lastCmdTime = millis();
        xSemaphoreGive(bleMutex);

        // In ra lệnh nhận được
        Serial.print("Lệnh nhận: ");
        Serial.println(cmd.c_str());       

        // ---- So sánh chuỗi tiếng Việt ----
        if (cmd.find("robot đi thẳng") != std::string::npos) {
          Serial.println("robot đi thẳng!");          
          sendBLE("OK");
          driveMotors(180,180);
        } 
        else if (cmd.find("robot qua trái") != std::string::npos) {
          Serial.println("robot qua trái!");
          sendBLE("OK");
          driveMotors(-180,180);
          vTaskDelay(pdMS_TO_TICKS(2000));
          driveMotors(0,0);
        } 
        else if (cmd.find("robot qua phải") != std::string::npos) {
          Serial.println("robot qua phải!");
          sendBLE("OK");
          driveMotors(180,-180);
          vTaskDelay(pdMS_TO_TICKS(2000));
          driveMotors(0,0);
        } 
        else if (cmd.find("robot đi lùi") != std::string::npos) {
          Serial.println("robot đi lùi!");
          sendBLE("OK");
          driveMotors(-180,-180);
        }
        else if (cmd.find("robot dừng") != std::string::npos) {
          Serial.println("robot dừng!");
          sendBLE("OK");
          driveMotors(0,0);
        } 
        else if (cmd.find("robot xoay") != std::string::npos) {
          Serial.println("robot xoay!");          
          driveMotors(250,-250);
        }
        else {
          Serial.println("Không nhận dạng được lệnh!");
          sendBLE("Error command!");
        }
      } else {
        xSemaphoreGive(bleMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

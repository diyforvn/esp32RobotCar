#include "web_config.h"
#include "config_manager.h"
#include <WiFi.h>
#include <WebServer.h>
#include "debug.h"

extern RobotConfig config;
static WebServer server(80);

void webTask(void *param) {
  for (;;) {
    server.handleClient();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}


void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>🤖 Robot Configuration</title>
    <style>
      /* Biến CSS */
      :root {
        --primary-color: #007bff; /* Blue */
        --success-color: #28a745; /* Green */
        --danger-color: #dc3545; /* Red */
        --text-color: #333;
        --bg-color: #f8f9fa; /* Light Gray */
        --card-bg: #ffffff;
        --border-color: #ced4da;
        --shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        --radius: 8px;
      }

      /* Thiết lập cơ bản */
      body {
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        background-color: var(--bg-color);
        color: var(--text-color);
        margin: 0;
        padding: 20px;
      }

      .container {
        max-width: 600px;
        margin: auto;
        background: var(--card-bg);
        padding: 20px;
        border-radius: var(--radius);
        box-shadow: var(--shadow);
      }

      h3 {
        text-align: center;
        color: var(--primary-color);
        margin-top: 0;
        border-bottom: 2px solid #eee;
        padding-bottom: 10px;
        margin-bottom: 20px;
      }

      /* Nhóm cấu hình */
      .config-group {
        margin-bottom: 20px;
        padding: 15px;
        border: 1px solid #e9ecef;
        border-radius: var(--radius);
        background-color: #f8f9fa;
      }
      .group-title {
        font-weight: bold;
        color: #495057;
        margin-bottom: 10px;
        display: block;
        border-bottom: 1px dashed #dee2e6;
        padding-bottom: 5px;
      }

      /* Trường nhập liệu */
      .form-field {
        display: flex;
        align-items: center;
        margin-bottom: 15px;
      }

      .form-field label {
        flex: 1;
        padding-right: 15px;
        font-weight: 500;
      }

      input[type=text], input[type=number] {
        flex: 2;
        padding: 10px;
        border: 1px solid var(--border-color);
        border-radius: var(--radius);
        transition: border-color 0.3s, box-shadow 0.3s;
        box-sizing: border-box;
      }
      input[type=text]:focus, input[type=number]:focus {
        border-color: var(--primary-color);
        box-shadow: 0 0 0 0.2rem rgba(0, 123, 255, 0.25);
        outline: none;
      }

      /* Checkbox */
      .checkbox-field {
        display: flex;
        align-items: center;
        margin-bottom: 15px;
        padding: 10px 0;
      }
      .checkbox-field label {
        flex: 1;
        font-weight: 500;
        margin-right: 15px;
      }
      .checkbox-container {
          flex: 2;
          display: flex;
          align-items: center;
      }
      input[type=checkbox] {
        transform: scale(1.5); /* Tăng kích thước checkbox */
        margin-right: 10px;
        cursor: pointer;
      }
      .checkbox-label-text {
        font-size: 0.9em;
        color: #6c757d;
      }


      /* Nút bấm */
      input[type=submit], button {
        color: white;
        border: none;
        padding: 12px;
        width: 100%;
        margin-top: 15px;
        cursor: pointer;
        border-radius: var(--radius);
        font-size: 16px;
        transition: background-color 0.3s, transform 0.1s;
        box-shadow: var(--shadow);
      }
      input[type=submit] {
        background: var(--success-color);
      }
      input[type=submit]:hover {
        background: #218838; /* Darker Green */
      }

      button.reboot {
        background: var(--danger-color);
      }
      button.reboot:hover {
        background: #c82333; /* Darker Red */
      }
      input[type=submit]:active, button:active {
        transform: translateY(1px);
      }

      /* Responsive */
      @media (max-width: 480px) {
        .form-field, .checkbox-field {
          flex-direction: column;
          align-items: flex-start;
        }
        .form-field label, .checkbox-field label {
          width: 100%;
          padding: 0 0 5px 0;
        }
        input[type=text], input[type=number] {
          width: 100%;
        }
        .checkbox-container {
            width: 100%;
            justify-content: flex-start;
        }
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h3>🤖 Robot Configuration</h3>
      <form action='/save' method='POST'>
)rawliteral";

  // ==== Các trường cấu hình ====
  // 1. PID Group
  html += "<div class='config-group'><span class='group-title'>PID Controller</span>";
  html += "<div class='form-field'><label for='Kp'>Kp:</label><input id='Kp' name='Kp' type='number' step='any' value='" + String(config.Kp) + "'></div>";
  html += "<div class='form-field'><label for='Ki'>Ki:</label><input id='Ki' name='Ki' type='number' step='any' value='" + String(config.Ki) + "'></div>";
  html += "<div class='form-field'><label for='Kd'>Kd:</label><input id='Kd' name='Kd' type='number' step='any' value='" + String(config.Kd) + "'></div>";
  html += "</div>"; // End PID Group

  // 2. Speed Group
  html += "<div class='config-group'><span class='group-title'>Motor Speed</span>";
  html += "<div class='form-field'><label for='baseSpeed'>BaseSpeed:</label><input id='baseSpeed' name='baseSpeed' type='number' value='" + String(config.baseSpeed) + "'></div>";
  html += "<div class='form-field'><label for='maxSpeed'>MaxSpeed:</label><input id='maxSpeed' name='maxSpeed' type='number' value='" + String(config.maxSpeed) + "'></div>";
  html += "</div>"; // End Speed Group

  // 3. Servo Group
  html += "<div class='config-group'><span class='group-title'>Servo Calibration</span>";
  html += "<div class='form-field'><label for='servoLeft'>ServoLeft:</label><input id='servoLeft' name='servoLeft' type='number' value='" + String(config.servoLeft) + "'></div>";
  html += "<div class='form-field'><label for='servoRight'>ServoRight:</label><input id='servoRight' name='servoRight' type='number' value='" + String(config.servoRight) + "'></div>";
  html += "<div class='form-field'><label for='servoCenter'>ServoCenter:</label><input id='servoCenter' name='servoCenter' type='number' value='" + String(config.servoCenter) + "'></div>";
  html += "<div class='form-field'><label for='servoStep'>ServoStep:</label><input id='servoStep' name='servoStep' type='number' value='" + String(config.servoStep) + "'></div>";
  html += "</div>"; // End Servo Group

  // 4. Obstacle/Scanning Group
  html += "<div class='config-group'><span class='group-title'>Obstacle & Scan</span>";
  html += "<div class='form-field'><label for='scanIntervalMs'>ScanInterval (ms):</label><input id='scanIntervalMs' name='scanIntervalMs' type='number' value='" + String(config.scanIntervalMs) + "'></div>";
  html += "<div class='form-field'><label for='obEnter'>ObstacleEnter (cm):</label><input id='obEnter' name='obEnter' type='number' value='" + String(config.obstacleEnterCm) + "'></div>";
  html += "<div class='form-field'><label for='obClear'>ObstacleClear (cm):</label><input id='obClear' name='obClear' type='number' value='" + String(config.obstacleClearCm) + "'></div>";
  html += "<div class='form-field'><label for='obFar'>obstacleFar (cm):</label><input id='obFar' name='obFar' type='number' value='" + String(config.obstacleFarCm) + "'></div>";
  html += "<div class='form-field'><label for='obNear'>obstacleNear (cm):</label><input id='obNear' name='obNear' type='number' value='" + String(config.obstacleNearCm) + "'></div>";
  html += "<div class='form-field'><label for='obCritical'>obstacleCritical (cm):</label><input id='obCritical' name='obCritical' type='number' value='" + String(config.obstacleCriticalCm) + "'></div>";
  html += "</div>"; // End Obstacle/Scanning Group

  // 5. Weights Group
  html += "<div class='config-group'><span class='group-title'>Behavior Weights (Float)</span>";
  html += "<div class='form-field'><label for='w_dist'>w_dist:</label><input id='w_dist' name='w_dist' type='number' step='any' value='" + String(config.w_dist) + "'></div>";
  html += "<div class='form-field'><label for='w_angle'>w_angle:</label><input id='w_angle' name='w_angle' type='number' step='any' value='" + String(config.w_angle) + "'></div>";
  html += "<div class='form-field'><label for='w_line'>w_line:</label><input id='w_line' name='w_line' type='number' step='any' value='" + String(config.w_line) + "'></div>";
  html += "</div>"; // End Weights Group

  // 6. Sensor Threshold
  html += "<div class='config-group'><span class='group-title'>Sensor Threshold</span>";
  html += "<div class='form-field'><label for='sensorThreshold'>SensorThreshold:</label><input id='sensorThreshold' name='sensorThreshold' type='number' value='" + String(config.sensorThreshold) + "'></div>";
  html += "</div>"; // End Sensor Threshold

  // 7. Checkbox Group
  html += "<div class='config-group'><span class='group-title'>Mode Selection</span>";

  // AutoMode
  html += "<div class='checkbox-field'><label for='autoMode'>Auto Mode:</label><span class='checkbox-container'><input id='autoMode' type='checkbox' name='autoMode' value='1'";
  if (config.autoMode) html += " checked";
  html += "></span></div>";

  // GamePad
  html += "<div class='checkbox-field'><label for='gamePad'>GamePad Control:</label><span class='checkbox-container'><input id='gamePad' type='checkbox' name='gamePad' value='1'";
  if (config.gamePad) html += " checked";
  html += "></span></div>";

  // sensorMode
  html += "<div class='checkbox-field'><label for='sensorMode'>Sensor Type:</label><span class='checkbox-container'><input id='sensorMode' type='checkbox' name='sensorMode' value='1'";
  if (config.sensorMode) html += " checked";
  html += "><span class='checkbox-label-text'>Check = Line Sensor, Uncheck = Ultrasonic</span></span></div>";

  html += "</div>"; // End Checkbox Group

  html += "<input type='submit' value='💾 Save Configuration'>";
  html += "</form>";

  // ===== Nút Reboot =====
  html += "<form action='/reboot' method='POST'>";
  html += "<button class='reboot' type='submit'>🔄 Reboot ESP32</button>";
  html += "</form>";

  html += "</div></body></html>";

  server.send(200, "text/html", html);
}


void handleSave() {
  if (server.hasArg("Kp")) config.Kp = server.arg("Kp").toFloat();
  if (server.hasArg("Ki")) config.Ki = server.arg("Ki").toFloat();
  if (server.hasArg("Kd")) config.Kd = server.arg("Kd").toFloat();

  if (server.hasArg("baseSpeed")) config.baseSpeed = server.arg("baseSpeed").toInt();
  if (server.hasArg("maxSpeed")) config.maxSpeed = server.arg("maxSpeed").toInt();

  if (server.hasArg("servoLeft")) config.servoLeft = server.arg("servoLeft").toInt();
  if (server.hasArg("servoRight")) config.servoRight = server.arg("servoRight").toInt();
  if (server.hasArg("servoCenter")) config.servoCenter = server.arg("servoCenter").toInt();
  if (server.hasArg("servoStep")) config.servoStep = server.arg("servoStep").toInt();

  if (server.hasArg("scanIntervalMs")) config.scanIntervalMs = server.arg("scanIntervalMs").toInt();
  if (server.hasArg("obEnter")) config.obstacleEnterCm = server.arg("obEnter").toInt();
  if (server.hasArg("obClear")) config.obstacleClearCm = server.arg("obClear").toInt();
  // THIẾU 3 THAM SỐ OBSTACLE ĐƯỢC THÊM TRONG handleRoot()
  if (server.hasArg("obFar")) config.obstacleFarCm = server.arg("obFar").toInt(); // THÊM
  if (server.hasArg("obNear")) config.obstacleNearCm = server.arg("obNear").toInt(); // THÊM
  if (server.hasArg("obCritical")) config.obstacleCriticalCm = server.arg("obCritical").toInt(); // THÊM
  // Hết THÊM

  if (server.hasArg("w_dist")) config.w_dist = server.arg("w_dist").toFloat();
  if (server.hasArg("w_angle")) config.w_angle = server.arg("w_angle").toFloat();
  if (server.hasArg("w_line")) config.w_line = server.arg("w_line").toFloat();

  if (server.hasArg("sensorThreshold")) config.sensorThreshold = server.arg("sensorThreshold").toInt();

  config.autoMode = server.hasArg("autoMode");
  config.gamePad  = server.hasArg("gamePad");
  config.sensorMode  = server.hasArg("sensorMode");

  saveConfig(); 

 
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Configuration Saved</title>
    <style>
      :root {
        --success-color: #28a745;
      }
      body {
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        background-color: #f8f9fa;
        text-align: center;
        padding-top: 50px;
      }
      .msg-card {
        max-width: 400px;
        margin: auto;
        background: white;
        padding: 30px;
        border-radius: 12px;
        box-shadow: 0 8px 15px rgba(0, 0, 0, 0.1);
        border: 1px solid #e9ecef;
      }
      .icon {
        font-size: 4em;
        color: var(--success-color);
        margin-bottom: 10px;
        display: block;
      }
      .msg-title {
        color: var(--success-color);
        font-size: 1.5em;
        font-weight: bold;
        margin-bottom: 10px;
      }
      .redirect-info {
        color: #6c757d;
        font-size: 0.9em;
      }
    </style>
  </head>
  <body>
    <div class='msg-card'>
      <span class='icon'>🎉</span>
      <div class='msg-title'>Configuration Saved Successfully!</div>
      <p class='redirect-info'>Redirecting to the configuration page in 3 seconds...</p>
    </div>
    <script>
      setTimeout(()=>{ window.location.href = '/'; }, 3000);
    </script>
  </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// ====== Xử lý reboot ======
void handleReboot() {
  //Serial.printf("Handle /save - method: %d\n", server.method());
  server.send(200, "text/html",
    "<html><body><h3>ESP32 đang khởi động lại...</h3>"
    "<script>setTimeout(()=>{window.location.href='/'},5000);</script>"
    "</body></html>");
  delay(500);
  ESP.restart();
}

// ====== Khởi tạo Web Config ======
void initWebConfig() {
  WiFi.softAP("RobotSetup", "123456789");
  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_ANY, handleSave);
  server.on("/reboot", HTTP_POST, handleReboot);

  server.begin();

  xTaskCreate(webTask, "WebServer", 4096, NULL, 1, NULL);
  Serial.println("WebConfig server started!");
}


void handleWebLoop() {
  server.handleClient();
}


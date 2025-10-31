#include "line_sensor.h"
#include "debug.h"


const int S0 = 34, S1 = 33, S2 = 35, S3 = 39, S4 = 36;
const int sensorPins[5] = {S0, S1, S2, S3, S4};
const int weights[5] = {-2, -1, 0, 1, 2};
extern TaskHandle_t hTaskLine;

void initLineSensors() {
    for (int i=0;i<5;i++) pinMode(sensorPins[i], INPUT);
    xTaskCreatePinnedToCore(Task_LineReader, "LineReader", 4096, NULL, 3, &hTaskLine, 1);
}


LineData readLineSensors() {
    LineData d;
    for (int i=0;i<5;i++) 
    {
        d.vals[i] = !digitalRead(sensorPins[i]); // đảo logic cho sensor. Line = 0 và mất line = 1
    }
    return d;
}


float computeLineError(const LineData &l) {
    int sumOn = 0; int wsum = 0;
    for (int i=0;i<5;i++) {
        if (l.vals[i]) { sumOn++; wsum += weights[i]; }
    }
    if (sumOn == 0) return 999.0;
    return (float)wsum / (float)sumOn;
}

void Task_LineReader(void *pv) {
  (void)pv;
  static float lastError = 0.0f;
  static int lostCount = 0;
  static float smoothErr = 0.0f;
  const float ERR_SMOOTH_ALPHA = 0.7f; // 0..1, nhỏ hơn = mượt hơn

  for (;;) {
    LineData ld = readLineSensors();
    float raw = computeLineError(ld); // raw, hoặc 999 nếu mất line

    float usedErr = 0.0f;
    if (raw > 500.0f) { // mất line
      lostCount++;
      float fake = (lastError > 0.0f) ? 1.0f : -1.0f;
      fake *= min(2.5f, 0.6f * lostCount); 
      usedErr = fake;
    } else {
      lostCount = 0;
      lastError = raw;
      usedErr = raw;
    }

    // smoothing 
    smoothErr = ERR_SMOOTH_ALPHA * smoothErr + (1.0f - ERR_SMOOTH_ALPHA) * usedErr;

    extern float g_lineError;
    g_lineError = smoothErr;

    vTaskDelay(pdMS_TO_TICKS(15)); // ~66 Hz
  }
}


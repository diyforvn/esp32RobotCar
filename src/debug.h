
#pragma once
#include <Arduino.h>

extern bool debugOn;
#define DEBUG_PRINT(x)   if(debugOn) Serial.print(x)
#define DEBUG_PRINTLN(x) if(debugOn) Serial.println(x)

#pragma once
#include <Arduino.h>


struct LineData { int vals[5]; };


void initLineSensors();
LineData readLineSensors();
float computeLineError(const LineData &l); // -2..2 or 999 if lost
void Task_LineReader(void *pv);
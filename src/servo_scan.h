#pragma once


void initServoScan();
void startAsyncScan();
// Task to be created: Task_ServoScan
void Task_ServoScan(void *pv);
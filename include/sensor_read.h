#ifndef SENSOR_READ_H
#define SENSOR_READ_H
#include <Arduino.h>

void initSensors();
void readSensors();
void setSensorOffsets(float tempOffset, float humOffset);
String autoCalibrateInternalSensor();
#endif

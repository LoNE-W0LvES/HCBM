#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

// Real-time readings
extern float internal_temp, internal_hum;
extern float ambient_temp, ambient_hum;

// Thresholds (configured)
extern float upper_temp_threshold;
extern float lower_temp_threshold;
extern float upper_hum_threshold;
extern float lower_hum_threshold;

// Actual lower limits
extern float lower_temp;
extern float lower_hum;

// Config
extern int mode;              // 1=Auto, 2=Manual, 3=Timer
extern String priority;       // "Temperature" or "Humidity"
extern int timer_value;       // minutes
extern bool timer_active;

// States
extern bool switch_manual;    // manual relay toggle
extern bool relay_state;      // current relay state
extern int fan_speed;         // RPM

// Buttons / UI
extern int current_menu;      // 1..7 (increased for tolerance params)
extern bool edit_mode;

extern unsigned long coolerTimerStart;
extern bool manual_override;

#endif
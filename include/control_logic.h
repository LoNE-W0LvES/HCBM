#ifndef CONTROL_LOGIC_H
#define CONTROL_LOGIC_H

void initControl();
void updateControl();
void setRelayState(bool state, int source);
unsigned long getProtectionRemaining();
bool isProtectionActive();
void resetStableTimer();  // NEW: Reset the stable temperature timer

#endif
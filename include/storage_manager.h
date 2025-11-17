#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>

void initStorage();
void saveConfiguration();
void loadConfiguration();
void saveLanguage(const char* lang);
String getLanguage();

#endif
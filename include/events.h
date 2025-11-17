// events.h
#ifndef EVENTS_H
#define EVENTS_H

#include <Arduino.h>

// Event types
enum EventType {
  EVENT_NONE = 0,
  EVENT_RELAY_CHANGED,
  EVENT_MODE_CHANGED,
  EVENT_PRIORITY_CHANGED,
  EVENT_THRESHOLDS_CHANGED,
  EVENT_TIMER_CHANGED,
  EVENT_SENSOR_UPDATE,
  EVENT_WIFI_STATUS_CHANGED,
  EVENT_BUTTON_PRESSED,
  EVENT_EDIT_MODE_CHANGED
};

// Event structure - Fixed to avoid String in union
struct SystemEvent {
  EventType type;
  unsigned long timestamp;
  
  // Relay event data
  bool relay_state;
  int relay_source;
  
  // Mode event data
  int mode_value;
  int mode_source;
  
  // Priority event data (use char array instead of String)
  char priority_value[16];
  
  // Edit mode data
  bool edit_entering;
  
  // Generic value
  int generic_value;
  
  // Constructor
  SystemEvent() : type(EVENT_NONE), timestamp(0), relay_state(false), 
                  relay_source(0), mode_value(0), mode_source(0),
                  edit_entering(false), generic_value(0) {
    priority_value[0] = '\0';
  }
};

// Event queue
class EventQueue {
private:
  static const int MAX_EVENTS = 16;
  SystemEvent events[MAX_EVENTS];
  int head = 0;
  int tail = 0;
  int count = 0;
  SemaphoreHandle_t mutex;

public:
  EventQueue() {
    mutex = xSemaphoreCreateMutex();
  }

  bool push(const SystemEvent& event) {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (count >= MAX_EVENTS) {
        xSemaphoreGive(mutex);
        return false; // Queue full
      }
      
      events[tail] = event;
      tail = (tail + 1) % MAX_EVENTS;
      count++;
      
      xSemaphoreGive(mutex);
      return true;
    }
    return false;
  }

  bool pop(SystemEvent& event) {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (count == 0) {
        xSemaphoreGive(mutex);
        return false; // Queue empty
      }
      
      event = events[head];
      head = (head + 1) % MAX_EVENTS;
      count--;
      
      xSemaphoreGive(mutex);
      return true;
    }
    return false;
  }

  bool isEmpty() {
    bool empty;
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      empty = (count == 0);
      xSemaphoreGive(mutex);
      return empty;
    }
    return true;
  }

  int getCount() {
    int cnt;
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      cnt = count;
      xSemaphoreGive(mutex);
      return cnt;
    }
    return 0;
  }
};

// Global event queue
extern EventQueue eventQueue;

// Helper functions to post events
inline void postRelayEvent(bool state, int source) {
  SystemEvent evt;
  evt.type = EVENT_RELAY_CHANGED;
  evt.timestamp = millis();
  evt.relay_state = state;
  evt.relay_source = source;
  eventQueue.push(evt);
}

inline void postModeEvent(int mode, int source) {
  SystemEvent evt;
  evt.type = EVENT_MODE_CHANGED;
  evt.timestamp = millis();
  evt.mode_value = mode;
  evt.mode_source = source;
  eventQueue.push(evt);
}

inline void postPriorityEvent(const String& priority) {
  SystemEvent evt;
  evt.type = EVENT_PRIORITY_CHANGED;
  evt.timestamp = millis();
  strncpy(evt.priority_value, priority.c_str(), sizeof(evt.priority_value) - 1);
  evt.priority_value[sizeof(evt.priority_value) - 1] = '\0';
  eventQueue.push(evt);
}

inline void postThresholdsEvent() {
  SystemEvent evt;
  evt.type = EVENT_THRESHOLDS_CHANGED;
  evt.timestamp = millis();
  eventQueue.push(evt);
}

inline void postTimerEvent() {
  SystemEvent evt;
  evt.type = EVENT_TIMER_CHANGED;
  evt.timestamp = millis();
  eventQueue.push(evt);
}

inline void postSensorEvent() {
  SystemEvent evt;
  evt.type = EVENT_SENSOR_UPDATE;
  evt.timestamp = millis();
  eventQueue.push(evt);
}

inline void postEditModeEvent(bool entering) {
  SystemEvent evt;
  evt.type = EVENT_EDIT_MODE_CHANGED;
  evt.timestamp = millis();
  evt.edit_entering = entering;
  eventQueue.push(evt);
}

inline void postButtonEvent(int button) {
  SystemEvent evt;
  evt.type = EVENT_BUTTON_PRESSED;
  evt.timestamp = millis();
  evt.generic_value = button;
  eventQueue.push(evt);
}

#endif
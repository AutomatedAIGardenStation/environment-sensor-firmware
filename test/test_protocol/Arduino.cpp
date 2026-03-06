#include "Arduino.h"

MockSerialProtocol Serial;

void delay(uint32_t ms) {}

uint32_t millis() { return 0; }

#ifndef ARDUINO_H
#define ARDUINO_H
#include <stdint.h>
#include <string.h>
#include <string>

// Mocks for Arduino Serial
class MockSerialProtocol {
public:
    std::string output;
    void begin(int baud) {}
    int available() { return 0; }
    int read() { return -1; }
    void print(const char* s) { output += s; }
    void println(const char* s) { output += s; output += "\n"; }
    void println() { output += "\n"; }
    operator bool() const { return true; }
    void clear() { output = ""; }
};

extern MockSerialProtocol Serial;

void delay(uint32_t ms);
uint32_t millis();

#endif

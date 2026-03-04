#ifndef AVERAGING_BUFFER_H
#define AVERAGING_BUFFER_H

#include <stdint.h>

/**
 * A fixed-size circular buffer that computes the arithmetic mean
 * of up to N samples. No heap allocation is performed.
 */
template <typename T, uint8_t N>
class AveragingBuffer {
public:
    AveragingBuffer() : head(0), count(0) {
        for (uint8_t i = 0; i < N; ++i) {
            buffer[i] = 0;
        }
    }

    /**
     * Adds a new sample to the buffer, overwriting the oldest sample
     * if the buffer is full.
     */
    void push(T sample) {
        buffer[head] = sample;
        head = (head + 1) % N;
        if (count < N) {
            count++;
        }
    }

    /**
     * Computes the arithmetic mean of all stored samples.
     * Returns 0.0f if the buffer is empty.
     */
    float average() const {
        if (count == 0) {
            return 0.0f;
        }

        // To avoid overflow, accumulate using a larger type or double.
        // float precision is usually sufficient for averaging up to a moderate number of small samples.
        // For general safety, use a double precision accumulator.
        double sum = 0.0;
        for (uint8_t i = 0; i < count; ++i) {
            sum += buffer[i];
        }

        return static_cast<float>(sum / count);
    }

    /**
     * Returns true if N samples have been pushed.
     */
    bool isFull() const {
        return count == N;
    }

private:
    T buffer[N];
    uint8_t head;
    uint8_t count;
};

#endif // AVERAGING_BUFFER_H

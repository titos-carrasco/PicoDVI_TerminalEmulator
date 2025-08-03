#ifndef _RCR_QUEUE_HPP_
#define _RCR_QUEUE_HPP_

#include <stdint.h>

template <typename T, uint8_t Capacity>
class Queue {
  public:
    Queue() : head(0), tail(0), count(0) {
    }

    bool push(const T &item) {
        if (isFull())
            return false;
        buffer[head] = item;
        head         = (head + 1) % Capacity;
        ++count;
        return true;
    }

    bool pop(T &item) {
        if (isEmpty())
            return false;
        item = buffer[tail];
        tail = (tail + 1) % Capacity;
        --count;
        return true;
    }

    bool peek(T &item) const {
        if (isEmpty())
            return false;
        item = buffer[tail];
        return true;
    }

    bool isEmpty() const {
        return count == 0;
    }

    bool isFull() const {
        return count == Capacity;
    }

    uint8_t size() const {
        return count;
    }

    void clear() {
        head = tail = count = 0;
    }

  private:
    T buffer[Capacity];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
};

#endif // _RCR_QUEUE_HPP_

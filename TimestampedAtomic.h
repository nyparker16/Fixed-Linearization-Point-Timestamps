#pragma once

#include <atomic>
#include <utility>
#include "Clock.h"

template <typename T>
class TimestampedAtomic {
    private:
        struct Node {
            T val;
            std::atomic<int> timestamp;
            Node* prev;
        };
        std::atomic<Node*> head;
        Clock* clock;
        void help_timestamp(Node* node);
    public:
        static constexpr int TBD = -1;
        TimestampedAtomic(T val, Clock* c);
        std::pair<T, int> load();
        int store(T newVal);
        std::pair<bool, int> CAS(T expected, T desired);
};

#include "TimestampedAtomic.tpp" 
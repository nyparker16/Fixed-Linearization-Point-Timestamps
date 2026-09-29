#pragma once

#include <atomic>
#include <optional>
#include <thread>
#include <vector>
#include <iostream>
#include <cassert>
#include "TimestampedAtomicNoLL.h"
#include "Clock.h" 

template <typename T>
class TreiberStack {
    private:
        struct Node {
            T value;
            Node* next;
        };
        Clock c;
        TimestampedAtomicNoLL<Node*> head;
    public:
        TreiberStack(): head(nullptr, &c) {}
        int push(T value);
        std::pair<std::optional<T>, int> pop();
};

#include "TreiberStack.tpp" 


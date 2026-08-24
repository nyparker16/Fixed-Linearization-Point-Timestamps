#pragma once

#include <atomic>
#include <optional>
#include <thread>
#include <vector>
#include <iostream>
#include <cassert>
#include "TimestampedAtomic.h"
#include "Clock.h" 

template <typename T>
class TreiberStack {
    private:
        struct Node {
            T value;
            Node* next;
        };
        TimestampedAtomic<Node*> head;
	Clock c;
    public:
        TreiberStack(): head(nullptr, &c) {}
        int push(T value);
        std::pair<std::optional<T>, int> pop();
};

#include "TreiberStack.tpp" 


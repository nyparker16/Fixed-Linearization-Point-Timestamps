#pragma once 

#include "TimestampedAtomic.h"

template <typename T>
void TimestampedAtomic<T>::help_timestamp(typename TimestampedAtomic<T>::Node* node) {
    if (node != nullptr && node -> timestamp.load() == TBD) {
        int expected = TBD;
        node->timestamp.compare_exchange_strong(expected, get_timestamp());
    }
}

template <typename T>
std::pair<T, int> TimestampedAtomic<T>::load() {
    int read_ts = get_timestamp();
    Node* curr = head.load();
    help_timestamp(curr);

    while (curr != nullptr && curr -> timestamp.load() > read_ts) {
        curr = curr -> prev;
    }

    return {curr -> val, read_ts};
}

template <typename T>
T TimestampedAtomic<T>::load_no_timestamping() {
    Node* curr = head.load();
    return curr->val;
}

template <typename T>
int TimestampedAtomic<T>::store(T newVal) {
    Node* newNode = new Node {newVal, TBD, nullptr};
    while(true) {
        Node* curr = head.load();
        help_timestamp(curr);

        newNode -> prev = curr;

        if (head.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return newNode -> timestamp.load();
        }
    }
}

template <typename T>
std::pair<bool, int> TimestampedAtomic<T>::CAS(T expected, T desired) {
    while(true) {
        int read_ts = get_timestamp();
        Node* curr = head.load();
        help_timestamp(curr);

        while (curr != nullptr && curr -> timestamp.load() > read_ts) {
            curr = curr -> prev;
        }

        if (curr -> val != expected) {
            return {false, read_ts};
        }

        Node* newNode = new Node{desired, TBD, curr};

        if (head.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return {true, next -> timestamp.load()};
        }
    }
}
#pragma once 

#include "TimestampedAtomicNoLL.h"

template <typename T> 
TimestampedAtomic<T>::TimestampedAtomic(T val, Clock* c) {
    clock = c;
    one_node = new Node{val, TBD};
    help_timestamp(head.load());
}

template <typename T>
void TimestampedAtomic<T>::help_timestamp(typename TimestampedAtomic<T>::Node* node) {
    if (node != nullptr && node -> timestamp.load() == TBD) {
        int expected = TBD;
        node -> timestamp.compare_exchange_strong(expected, clock -> get_timestamp());
    }
}

template <typename T>
std::pair<T, int> TimestampedAtomic<T>::load() {
    int read_ts = clock -> get_timestamp();
    Node* curr = one_node.load();
    help_timestamp(curr);
    return (read_ts > curr -> timestamp) ? {curr -> val, read_ts} : {curr -> val, curr -> timestamp}; 
    // Non-unique timestamps in the case when curr -> timestamp > read_ts
}

template <typename T>
T TimestampedAtomic<T>::load_no_timestamping() {
    Node* curr = oneNode.load();
    return curr -> val;
}

template <typename T>
int TimestampedAtomic<T>::store(T newVal) {
    Node* newNode = new Node {newVal, TBD};
    while(true) {
        Node* curr = oneNode.load();
        help_timestamp(curr);

        if (oneNode.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return newNode -> timestamp.load();
        }
    }
}

template <typename T>
std::pair<bool, int> TimestampedAtomic<T>::CAS(T expected, T desired) {
    Node* newNode = new Node{desired, TBD, curr};
    while(true) {
        int read_ts = clock -> get_timestamp();
        Node* curr = oneNode.load();
        help_timestamp(curr);

        Node* max = (read_ts > curr -> timestamp) ? {curr -> val, read_ts} : {curr -> val, curr -> timestamp};        

        if (max -> val != expected) {
            delete newNode;
            return {false, read_ts};
        }

        if (oneNode.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return {true, newNode -> timestamp.load()};
        }
    }
}
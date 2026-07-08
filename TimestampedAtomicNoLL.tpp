#pragma once 

#include "TimestampedAtomicNoLL.h"

template <typename T> 
TimestampedAtomicNoLL<T>::TimestampedAtomicNoLL(T val, Clock* c) {
    clock = c;
    one_node = new Node{val, TBD};
    help_timestamp(one_node.load());
}

template <typename T>
void TimestampedAtomicNoLL<T>::help_timestamp(typename TimestampedAtomicNoLL<T>::Node* node) {
    if (node != nullptr && node -> timestamp.load() == TBD) {
        int expected = TBD;
        node -> timestamp.compare_exchange_strong(expected, clock -> get_timestamp());
    }
}

template <typename T>
std::pair<T, int> TimestampedAtomicNoLL<T>::load() {
    int read_ts = clock -> get_timestamp();
    Node* curr = one_node.load();
    help_timestamp(curr);
    int ts = curr->timestamp.load();
    return (read_ts > ts) ? std::make_pair(curr->val, read_ts)
                           : std::make_pair(curr->val, ts);
    // Non-unique timestamps in the case when curr -> timestamp > read_ts
}

template <typename T>
T TimestampedAtomicNoLL<T>::load_no_timestamping() {
    Node* curr = one_node.load();
    return curr -> val;
}

template <typename T>
int TimestampedAtomicNoLL<T>::store(T newVal) {
    Node* newNode = new Node {newVal, TBD};
    while(true) {
        Node* curr = one_node.load();
        help_timestamp(curr);

        if (one_node.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return newNode -> timestamp.load();
        }
    }
}

template <typename T>
std::pair<bool, int> TimestampedAtomicNoLL<T>::CAS(T expected, T desired) {
    Node* newNode = new Node{desired, TBD};
    while(true) {
        int read_ts = clock -> get_timestamp();
        Node* curr = one_node.load();
        help_timestamp(curr);

        if (curr -> val != expected) {
            delete newNode;
            return {false, read_ts};
        }

        if (one_node.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return {true, newNode -> timestamp.load()};
        }
    }
}
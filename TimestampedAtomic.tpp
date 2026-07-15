#include "TimestampedAtomic.h"

template <typename T> 
TimestampedAtomic<T>::TimestampedAtomic(T val, Clock* c) {
    clock = c;
    head = new Node{val, TBD, nullptr};
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
    Node* curr = head.load();
    help_timestamp(curr);

    while (curr != nullptr && curr -> timestamp.load() > read_ts) {
        curr = curr -> prev;
    }

    return {curr -> val, read_ts};
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
    Node* newNode = new Node{desired, TBD, nullptr};
    while(true) {
        int read_ts = clock -> get_timestamp();
        Node* curr = head.load();
        help_timestamp(curr);

        while (curr != nullptr && curr -> timestamp.load() > read_ts) {
            curr = curr -> prev;
        }

        if (curr -> val != expected) {
            delete newNode;
            return {false, read_ts};
        }

        newNode -> prev = curr;

        if (head.compare_exchange_weak(curr, newNode)) {
            help_timestamp(newNode);
            return {true, newNode -> timestamp.load()};
        }
    }
}
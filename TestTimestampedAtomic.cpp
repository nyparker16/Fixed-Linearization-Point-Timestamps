#include "Clock.h"
#include "TimestampedAtomic.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <cassert>

template <typename T>
void thread_fn(TimestampedAtomic<T>* ta, int operation_count) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> operation(0, 2);
    std::uniform_int_distribution<int> value(-50, 50);

    T loadValue = ta -> load().first;
    bool CAS_result = false;

    for (int i = 0; i < operation_count; i++) {
        int op = operation(rng);
        int v = value(rng);
        int timestamp = TimestampedAtomic<T>::TBD; 
        switch (op) {
            case 0: {
                // load
                std::pair<T, int> p = ta -> load();
                loadValue = p.first;
                timestamp = p.second;
                break;
            }
            case 1: {
                // store
                timestamp = ta -> store(v);
                break;
            }
            case 2: {
                // CAS
                std::pair<bool, int> p = ta -> CAS(loadValue, v);
                CAS_result = p.first;
                timestamp = p.second;
                break;
            }
            default: {
                // unreachable
                assert(false && "unreachable");
            }
        }
    }
}

int main(int argc, char** argv) {
    
    if (argc < 3) {
        std::cerr << "Too few arguments." << std::endl;
        std::cerr << "Usage: " << argv[0] << " thread_count operation_count" << std::endl;
        return 1;
    }

    const int thread_count = atoi(argv[1]);
    const int operation_count = atoi(argv[2]);

    Clock c;

    TimestampedAtomic<int> t1(0, &c);
    // TimestampedAtomic<int> t2(100, &c);
    // TimestampedAtomic<char> t3('a', &c);
    // TimestampedAtomic<std::string> t4("aaa", &c);

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_fn<int>, &t1, operation_count);
    }

    for (int i = 0; i < thread_count; i++) {
        threads[i].join();
        std::cerr << "Thread " << i << " finished." << std::endl;
    }

    return 0;
}
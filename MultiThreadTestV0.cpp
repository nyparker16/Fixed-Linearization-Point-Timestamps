#include "Clock.h"
#include "TimestampedAtomic.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <cassert>
#include <atomic>
#include <array>

enum Operation {
    LOAD,
    STORE,
    CAS,
    UNINITIALIZED
};

struct LogEntry {
    Operation o;
    int first_val, second_val;
};
constexpr LogEntry UNINITIALIZED_LOG{UNINITIALIZED, 0, 0};

constexpr int LOG_CAPACITY = 1000000;

std::array<LogEntry, LOG_CAPACITY> operation_log;

template <typename T>
void thread_fn(TimestampedAtomic<T>* ta, int operation_count) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> operation(0, 2);
    std::uniform_int_distribution<int> value(-50, 50);

    for (int i = 0; i < operation_count; i++) {
        int op = operation(rng);
        switch (op) {
            case 0: {
                std::pair<T, int> p = ta -> load();
                int v = p.first;
                int timestamp = p.second;

                if (timestamp >= LOG_CAPACITY) {
                    std::cerr << "Log capacity reached. Too many operations." << std::endl;
                    exit(1);
                }
                operation_log[timestamp] = LogEntry{LOAD, v, 0};

                break;
            }
            case 1: {
                int v = value(rng);
                int timestamp = ta -> store(v);

                if (timestamp >= LOG_CAPACITY) {
                    std::cerr << "Log capacity reached. Too many operations." << std::endl;
                    exit(1);
                }
                operation_log[timestamp] = LogEntry{STORE, v, 0};

                break;
            }
            case 2: {
                int expected = value(rng);
                int desired = value(rng);
                std::pair<bool, int> p = ta -> CAS(expected, desired);
                bool success = p.first;
                int timestamp = p.second;

                if (timestamp >= LOG_CAPACITY) {
                    std::cerr << "Log capacity reached. Too many operations." << std::endl;
                    exit(1);
                }
                operation_log[timestamp] = LogEntry{CAS, expected, desired};

                break;
            }
            default: {
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

    TimestampedAtomic<int> t1{0, &c};
    // TimestampedAtomic<int> t2(100, &c);
    // TimestampedAtomic<char> t3('a', &c);
    // TimestampedAtomic<std::string> t4("aaa", &c);

    for (int i = 0; i < LOG_CAPACITY; i++) {
        operation_log[i] = UNINITIALIZED_LOG;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_fn<int>, &t1, operation_count);
    }

    for (int i = 0; i < thread_count; i++) {
        threads[i].join();
        std::cerr << "Thread " << i << " finished." << std::endl;
    }

    std::atomic<int> atomic_var{0};

    for (int i = 0; i < LOG_CAPACITY; i++) {
        if (operation_log[i].o == UNINITIALIZED) {
            continue;
        }
        Operation op = operation_log[i].o;
        switch (op) {
            case LOAD: {
                int atomic_val = atomic_var.load();
                if (operation_log[i].first_val != atomic_val) {
                    assert(false && "load failed");
                }
                break;
            }
            case STORE: {
                atomic_var.store(operation_log[i].first_val);
                break;
            }
            case CAS: {
                int expected = operation_log[i].first_val;
                atomic_var.compare_exchange_strong(expected, operation_log[i].second_val);
                break;
            }
            default: {
                assert(false && "unreachable");
            }
        }
    }

    // Main Test
    int test_val = t1.load().first;
    if (test_val == atomic_var.load()) {
        std::cout << "Success!" << std::endl;
    } else {
        std::cout << "Failure" << std::endl;
    }
    std::cout << "TimestampedAtomic        Standard Atomic" << std::endl;
    std::cout << test_val << "                       " << atomic_var.load() << std::endl;

    return 0;
}
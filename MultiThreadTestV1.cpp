#include "Clock.h"
#include "TimestampedAtomicNoLL.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <cassert>
#include <atomic>
#include <array>
#include <algorithm>

enum Operation {
    LOAD,
    STORE,
    CAS,
    UNINITIALIZED
};

struct LogEntry {
    Operation o;
    int first_val, second_val;
    bool CAS_success;
    int timestamp;
};
constexpr LogEntry UNINITIALIZED_LOG{UNINITIALIZED, 0, 0};

int operation_rank(Operation o) {
    switch (o) {
        case STORE : return 0;
        case CAS : return 0;
        case LOAD : return 1;
        case UNINITIALIZED: return 2;
        default : 
            assert(false && "unreachable");
            return 9999;
    }
}

constexpr int LOG_CAPACITY = 1000000;

std::array<LogEntry, LOG_CAPACITY> total_operation_log;

template <typename T>
void thread_fn(TimestampedAtomicNoLL<T>* ta, int operation_count, int thread_num) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> operation(0, 2);
    std::uniform_int_distribution<int> value(-5, 5);

    for (int i = 0; i < operation_count; i++) {
        int op = operation(rng);
        int index = thread_num * operation_count + i;
        switch (op) {
            case 0: {
                std::pair<T, int> p = ta -> load();
                int v = p.first;
                int timestamp = p.second;

                total_operation_log[index] = LogEntry{LOAD, v, 0, false, timestamp};

                break;
            }
            case 1: {
                int v = value(rng);
                int timestamp = ta -> store(v);

                total_operation_log[index] = LogEntry{STORE, v, 0, false, timestamp};

                break;
            }
            case 2: {
                int expected = value(rng);
                int desired = value(rng);
                std::pair<bool, int> p = ta -> CAS(expected, desired);
                bool success = p.first;
                int timestamp = p.second;

                total_operation_log[index] = LogEntry{CAS, expected, desired, success, timestamp};

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

    TimestampedAtomicNoLL<int> t1{0, &c};
    // TimestampedAtomicNoLL<int> t2(100, &c);
    // TimestampedAtomicNoLL<char> t3('a', &c);
    // TimestampedAtomicNoLL<std::string> t4("aaa", &c);

    if (LOG_CAPACITY < thread_count * operation_count) {
        std::cerr << "Too many operations per thread" << std::endl;
        exit(1);
    }

    for (int i = 0; i < LOG_CAPACITY; i++) {
        total_operation_log[i] = UNINITIALIZED_LOG;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_fn<int>, &t1, operation_count, i);
    }

    for (int i = 0; i < thread_count; i++) {
        threads[i].join();
        std::cerr << "Thread " << i << " finished." << std::endl;
    }

    std::sort(total_operation_log.begin(), total_operation_log.end(), 
                [](const LogEntry& a, const LogEntry& b) {
                    if (a.timestamp != b.timestamp) {
                        return a.timestamp < b.timestamp;
                    }
                    if (a.timestamp == b.timestamp && a.o == CAS && b.o == CAS) {
                        return a.CAS_success > b.CAS_success;
                    } else {
                        return operation_rank(a.o) < operation_rank(b.o);
                    }
                });

    std::atomic<int> atomic_var{0};

    for (int i = 0; i < LOG_CAPACITY; i++) {
        if (total_operation_log[i].o == UNINITIALIZED) {
            continue;
        }
        Operation op = total_operation_log[i].o;
        switch (op) {
            case LOAD : {
                int atomic_val = atomic_var.load();
                // std::cout << std::cout << "LOAD: std atomic: " << atomic_val << " ,TimestampedAtomic: " << total_operation_log[i].first_val 
                //        << " ,ts: " << total_operation_log[i].timestamp << std::endl;
                if (total_operation_log[i].first_val != atomic_val) {
                    assert(false && "load failed");
                }
                break;
            }
            case STORE : {
                // std::cout << "STORE: " << total_operation_log[i].first_val 
                //        << " ,ts: " << total_operation_log[i].timestamp << std::endl;
                atomic_var.store(total_operation_log[i].first_val);
                break;
            }
            case CAS : {
                int expected = total_operation_log[i].first_val;
                bool success = atomic_var.compare_exchange_strong(expected, total_operation_log[i].second_val);
                // std::cout << std::boolalpha;
                // std::cout << "CAS: expected: " << total_operation_log[i].first_val << " , desired: " << total_operation_log[i].second_val 
                //        << " , std atomic: " << success << " ,TimestampedAtomic: " << total_operation_log[i].CAS_success
                //        << " ,ts: " << total_operation_log[i].timestamp << std::endl;
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
    std::cout << "TimestampedAtomicNoLL        Standard Atomic" << std::endl;
    std::cout << test_val << "                           " << atomic_var.load() << std::endl;

    return 0;
}
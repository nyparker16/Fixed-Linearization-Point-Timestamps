#include "Clock.h"
#include "TreiberStack.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <cassert>
#include <atomic>
#include <array>
#include <algorithm>
#include <stack>

enum Operation {
    POP,
    PUSH,
    UNINITIALIZED
};

struct LogEntry {
    Operation o;
    int val;
    int timestamp;
};
constexpr LogEntry UNINITIALIZED_LOG{UNINITIALIZED, 0, 0};

constexpr int LOG_CAPACITY = 1000000;

std::array<LogEntry, LOG_CAPACITY> total_operation_log;

template <typename T>
void thread_fn(TreiberStack<T>* t, int operation_count, int thread_num) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> operation(0, 1);
    std::uniform_int_distribution<int> value(1, 10);

    for (int i = 0; i < operation_count; i++) {
        int op = operation(rng);
        int index = thread_num * operation_count + i;
        switch (op) {
            case 0: {
                std::pair<std::optional<T>, int> p = t -> pop();
                int v = p.first.value_or(-1); 
                int timestamp = p.second;

                total_operation_log[index] = LogEntry{POP, v, timestamp};

                break;
            }
            case 1: {
                int v = value(rng);
                int timestamp = t -> push(v);

                total_operation_log[index] = LogEntry{PUSH, v, timestamp};

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

    TreiberStack<int> t1;
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
                    return a.timestamp < b.timestamp;
                });

    std::stack<int> atomic_var;

    for (int i = 0; i < LOG_CAPACITY; i++) {
        if (total_operation_log[i].o == UNINITIALIZED) {
            continue;
        }
        Operation op = total_operation_log[i].o;
        switch (op) {
            case POP : {
                int atomic_val;
                if (atomic_var.empty()) {
                    atomic_val = -1;
                } else {
                    atomic_val = atomic_var.top();
                    atomic_var.pop();
                }
                // std::cout << "POP: std stack: " << atomic_val << " ,TreiberStack: " << total_operation_log[i].val 
                //        << " ,ts: " << total_operation_log[i].timestamp << std::endl;
                if (total_operation_log[i].val != atomic_val) {
                    assert(false && "pop failed");
                }
                break;
            }
            case PUSH : {
                // std::cout << "PUSH: " << total_operation_log[i].val 
                //        << " ,ts: " << total_operation_log[i].timestamp << std::endl;
                atomic_var.push(total_operation_log[i].val);
                break;
            }
            default: {
                assert(false && "unreachable");
            }
        }
    }
    
    std::cout << "Success!" << std::endl;

    return 0;
}
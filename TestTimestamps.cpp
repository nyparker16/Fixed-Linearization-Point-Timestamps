#include "Clock.h"
#include "TimestampedAtomic.h"
#include "TimestampedAtomicNoLL.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <cassert>

enum Operation {
    LOAD,
    STORE,
    CAS,
    UNINITIALIZED
};

constexpr int MAX_OPERATIONS = 100000;

struct LogEntry {
    Operation o;
    int start, end;
    int timestamp;
};
constexpr LogEntry UNINITIALIZED_LOG = LogEntry{UNINITIALIZED, 0, 0, 0};

std::array<LogEntry, MAX_OPERATIONS> timestamp_log;

template <typename T>
void thread_fn(TimestampedAtomicNoLL<T>* ta, int operation_count, int thread_num) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> operation(0, 2);
    std::uniform_int_distribution<int> value(-50, 50);

    T loadValue = ta -> load().first;
    bool CAS_result = false;

    for (int i = 0; i < operation_count; i++) {
        int op = operation(rng);
        int v = value(rng);
        int timestamp, end_timestamp = -1;
        int index = thread_num * operation_count + i;

        int start_timestamp = __builtin_ia32_rdtsc();
        switch (op) {
            case 0: {
                // load
                std::pair<T, int> p = ta -> load();
                loadValue = p.first;
                timestamp = p.second;
                end_timestamp = __builtin_ia32_rdtsc();

                timestamp_log[i] = LogEntry{LOAD, start_timestamp, end_timestamp, timestamp};

                break;
            }
            case 1: {
                // store
                timestamp = ta -> store(v);
                end_timestamp = __builtin_ia32_rdtsc();

                timestamp_log[i] = LogEntry{STORE, start_timestamp, end_timestamp, timestamp};
                break;
            }
            case 2: {
                // CAS
                std::pair<bool, int> p = ta -> CAS(loadValue, v);
                CAS_result = p.first;
                timestamp = p.second;
                end_timestamp = __builtin_ia32_rdtsc();

                timestamp_log[i] = LogEntry{CAS, start_timestamp, end_timestamp, timestamp};
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

    TimestampedAtomicNoLL<int> t1(0, &c);
    // TimestampedAtomic<int> t2(100, &c);
    // TimestampedAtomic<char> t3('a', &c);
    // TimestampedAtomic<std::string> t4("aaa", &c);

    if (MAX_OPERATIONS < thread_count * operation_count) {
        std::cerr << "Too many operations" << std::endl;
        exit(1);
    }

    for (int i = 0; i < MAX_OPERATIONS; i++) {
        timestamp_log[i] = UNINITIALIZED_LOG;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_fn<int>, &t1, operation_count, i);
    }

    for (int i = 0; i < thread_count; i++) {
        threads[i].join();
        std::cerr << "Thread " << i << " finished." << std::endl;
    }

    int num_failures = 0;
    for (int i = 0; i < MAX_OPERATIONS; i++) {
        if (timestamp_log[i].o == UNINITIALIZED) {
            continue;
        }
        for (int j = 0; j < MAX_OPERATIONS; j++) {
            if (timestamp_log[i].o == UNINITIALIZED) {
                continue;
            }
            if (timestamp_log[i].timestamp <= timestamp_log[j].timestamp && timestamp_log[i].start >= timestamp_log[j].end) {
                // std::cout << timestamp_log[i].timestamp << " " << timestamp_log[j].timestamp << " " 
                //         << timestamp_log[i].start << " " << timestamp_log[j].end << std::endl;
                num_failures++;
            } else if (timestamp_log[i].timestamp >= timestamp_log[j].timestamp && timestamp_log[j].start >= timestamp_log[i].end) {
                // std::cout << timestamp_log[j].timestamp << " " << timestamp_log[i].timestamp << " " 
                //         << timestamp_log[j].start << " " << timestamp_log[i].end << std::endl;
                num_failures++;
            }
        }
    }
    if (num_failures == 0) {
        std::cout << "Success!" << std::endl;
    } else {
        std::cout << "Failed" << std::endl;
    }
    std::cout << "Number of failures: " << num_failures << std::endl;

    return 0;
}
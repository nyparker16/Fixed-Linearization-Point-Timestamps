#pragma once

#include <atomic>
#include <array>
#include <vector>
#include "TimestampedAtomicNoLL.h"

template <typename T>
class EBR {
    private:
        static constexpr int MAX_THREADS = 128;
        static constexpr int INACTIVE = -1;
        std::atomic<int> global_epoch{0};
        std::atomic<int> num_registered{0};
        std::array<std::atomic<int>, MAX_THREADS> thread_epoch;
        std::array<std::vector<std::pair<int, typename TimestampedAtomicNoLL<T>::Node*>>, MAX_THREADS> garbage;

        static inline thread_local int thread_id = INACTIVE;
    public:
        EBR();
        void register_thread();
        void pin();
        void unpin();
        void retire(typename TimestampedAtomicNoLL<T>::Node* n);
        void try_advance();
};

#include "EBR.tpp"




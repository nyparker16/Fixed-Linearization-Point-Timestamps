#include <iostream>
#include <cassert>
#include "EBR.h"

EBR::EBR() {
    // NOTE: the constructor must fully complete before any thread uses this API
    for (auto& te : thread_epoch) {
        te.store(INACTIVE);
    }
}

void EBR::register_thread() {
    int id = num_registered.fetch_add(1);
    assert(id < MAX_THREADS);
    thread_id = id;
}

void EBR::pin() {
    assert(thread_epoch[thread_id] != INACTIVE);
    thread_epoch[thread_id].store(e);
}

void EBR::unpin() {
    assert(thread_epoch[thread_id] != INACTIVE);
    thread_epoch[thread_id].store(INACTIVE);
    try_advance();
}

template <typename T>
void EBR::retire(typename TimestampedAtomicNoLL<T>::Node* n) {
    int e = global_epoch.load();
    garbage.push_back(std::make_pair(e, n));
}

void EBR::try_advance() {

}
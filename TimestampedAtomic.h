#include <atomic>
#include <utility>
#include "Clock.h"

template <typename T>
class TimestampedAtomic {
    private:
        struct Node {
            T val;
            std::atomic<int> timestamp;
            Node* prev;
        };
        std::atomic<Node*> head;
        Clock* clock;
        const int TBD = -1;
        void help_timestamp(Node* node);
    public:
        TimestampedAtomic(T val, Clock* c);
        std::pair<T, int> load();
        T load_no_timestamping();
        int store(T newVal);
        std::pair<bool, int> CAS(T expected, T desired);
};

#include "TimestampedAtomic.tpp" 
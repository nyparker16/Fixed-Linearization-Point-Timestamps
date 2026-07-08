#include <atomic>
#include <utility>
#include <Clock.h>

template <typename T>
class TimestampedAtomicNoLL {
    private:
        struct Node {
            T val;
            std::atomic<int> timestamp;
        };
        std::atomic<Node*> one_node;
        Clock* clock;
        const int TBD = -1;
        void help_timestamp(Node* node);
    public:
        TimestampedAtomicNoLL(T val, Clock* c);
        std::pair<T, int> load();
        T load_no_timestamping();
        int store(T newVal);
        std::pair<bool, int> CAS(T expected, T desired);
};

#include "TimestampedAtomicNoLL.tpp" 
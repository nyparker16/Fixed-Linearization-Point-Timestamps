#include <atomic>
#include <utility>

template <typename T>
class TimestampedAtomic {
    private:
        struct Node {
            T val;
            std::atomic<int> timestamp;
            Node* prev;
        };

        std::atomic<Node*> head;
        const int TBD = -1;

        void help_timestamp(Node* node);
        int get_timestamp();

    public:
        std::pair<T, int> load();
        T load_no_timestamping();
        int store(T newVal);
        std::pair<bool, int> CAS(T expected, T desired);
};

#include "TimestampedAtomic.tpp" 
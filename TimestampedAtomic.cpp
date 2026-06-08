#include <atomic>
template <typename T>

class TimestampedAtomic {
    struct Node {
        T val;
        std::atomic<int> timestamp;
        Node* prev;
    };

    std::atomic<Node*> head;
    const int TBD = -1;

    void help_timestamp(Node* node) {
        if (node != nullptr && node -> timestamp.load() == TBD) {
            node -> timestamp.CAS(TBD, get_timestamp());
        }
    }

    <T, int> load() {
        int read_ts = get_timestamp();
        Node* curr = head.load();
        help_timestamp(curr);

        while (curr != nullptr && curr -> timestamp.load() > read_ts) {
            curr = curr -> prev;
        }

        return {curr -> val, read_ts};
    }

    int store(T newVal) {
        while(true) {
            Node* curr = head.load();
            help_timestamp(curr);

            Node* next = new Node{newVal, TBD, curr};

            if (head.CAS(curr, next)) {
                help_timestamp(next);
                return next_timestamp.load();
            }
        }
    }

    <bool, int> CAS(T expected, T desired) {
        while(true) {
            Node* curr = head.load();
            help_timestamp(curr);

            if (curr -> val != expected) {
                return {false, curr -> timestamp.load()};
            }

            Node* next = new Node{desired, TBD, curr};

            if (head.CAS(curr, next)) {
                help_timestamp(next);
                return {true, next -> timestamp.load()};
            }
        }
    }
};
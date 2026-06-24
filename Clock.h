#include <atomic>

class Clock {
    private:
        std::atomic<int> timestamp;
    public:
        Clock() { timestamp = 0; }
        int get_timestamp() {
            int ts = timestamp.load();
            timestamp.compare_exchange_strong(ts, ts+1);
            return ts;
        }
};
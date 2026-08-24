#include "TreiberStack.h"

template <typename T>
int TreiberStack<T>::push(T value) {
    Node* new_node = new Node{value, nullptr};
    new_node->next = head.load().first;

    std::pair<bool, int> result;

    // Linearization point: this CAS.
    while (!(result = head.CAS(new_node->next, new_node)).first) {
        new_node->next = head.load().first;
    }
    
    return result.second;
}

template <typename T>
std::pair<std::optional<T>, int> TreiberStack<T>::pop() {
    std::pair<Node*, int> load_result = head.load();
    Node* old_head = load_result.first;
    int timestamp = load_result.second;

    while (old_head != nullptr) {
        std::pair<bool, int> cas_result;

        // Linearization point: this CAS.
        if ((cas_result = head.CAS(old_head, old_head->next)).first) {
            T val = old_head -> value;
            timestamp = cas_result.second;
            return std::make_pair(val, timestamp);
        }

        load_result = head.load();
        old_head = load_result.first;
        timestamp = load_result.second;
    }

    return std::pair<std::optional<T>, int>(std::nullopt, timestamp);
}
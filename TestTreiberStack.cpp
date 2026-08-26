#include "TreiberStack.h"
#include <iostream>

int main() {
    TreiberStack<int> ts;

    ts.push(1);
    ts.push(2);
    ts.push(3);

    std::cout << "Expected: 3, Actual: " << ts.pop().first.value_or(-1) << std::endl;
    std::cout << "Expected: 2, Actual: " << ts.pop().first.value_or(-1) << std::endl;
    std::cout << "Expected: 1, Actual: " << ts.pop().first.value_or(-1) << std::endl;

    return 0;
}
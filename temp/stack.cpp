#include "stack.h"

void Stack::push(const std::string& symbol) {
    elements.push_back(symbol);
}

std::string Stack::pop() {
    if (elements.empty()) return "";
    std::string topElement = elements.back();
    elements.pop_back();
    return topElement; // Return what was popped
}

std::string Stack::top() const {
    if (elements.empty()) return "";
    return elements.back();
}

bool Stack::isEmpty() const {
    return elements.empty();
}

std::string Stack::getStackString() const {
    std::string result = "";
    // std::vector naturally iterates from index 0 (bottom) to end (top)
    for (const std::string& s : elements) {
        result += s + " ";
    }
    return result;
}
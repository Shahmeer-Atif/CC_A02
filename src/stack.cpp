#include "stack.h"

void Stack::push(TreeNode* node) { elements.push_back(node); }

TreeNode* Stack::pop() {
    if (elements.empty()) return nullptr;
    TreeNode* topElement = elements.back();
    elements.pop_back();
    return topElement;
}

TreeNode* Stack::top() const {
    if (elements.empty()) return nullptr;
    return elements.back();
}

bool Stack::isEmpty() const { return elements.empty(); }

std::string Stack::getStackString() const {
    std::string result = "";
    for (TreeNode* node : elements) result += node->symbol + " ";
    return result;
}
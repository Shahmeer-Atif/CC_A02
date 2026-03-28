#include "tree.h"
#include <iostream>

void printTree(TreeNode* node, int depth) {
    if (node == nullptr) return;
    for (int i = 0; i < depth; ++i) std::cout << "  |";
    if (depth > 0) std::cout << "--";
    std::cout << node->symbol << "\n";
    for (TreeNode* child : node->children) {
        printTree(child, depth + 1);
    }
}

void destroyTree(TreeNode* node) {
    if (node == nullptr) return;
    for (TreeNode* child : node->children) {
        destroyTree(child);
    }
    delete node;
}
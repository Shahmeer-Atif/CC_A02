#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>

struct TreeNode {
    std::string symbol;
    std::vector<TreeNode*> children;
    TreeNode(const std::string& s) : symbol(s) {}
};

void printTree(TreeNode* node, int depth = 0);
void destroyTree(TreeNode* node);

#endif
#ifndef STACK_H
#define STACK_H

#include <vector>
#include <string>
#include "tree.h"

class Stack {
private:
    std::vector<TreeNode*> elements;
public:
    void push(TreeNode* node);
    TreeNode* pop();
    TreeNode* top() const;
    bool isEmpty() const;
    std::string getStackString() const; 
};

#endif
#ifndef STACK_H
#define STACK_H

#include <vector>
#include <string>

class Stack {
private:
    std::vector<std::string> elements;

public:
    void push(const std::string& symbol);
    std::string pop();
    std::string top() const;
    bool isEmpty() const;
    
    // Required by assignment: Show stack from bottom to top for the trace
    std::string getStackString() const; 
};

#endif // STACK_H
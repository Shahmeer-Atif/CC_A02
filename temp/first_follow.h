#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"
#include <map>
#include <set>
#include <string>

class FirstFollow {
public:
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;

    // Pass the grammar by reference so we can read its rules
    void computeFirstSets(const Grammar& grammar);
    
    // Print in tabular format as requested by the assignment
    void printFirstSets() const;

    // Add these under computeFirstSets
    void computeFollowSets(const Grammar& grammar);
    void printFollowSets() const;
private:
    // Helper to check if a symbol can derive epsilon
    bool derivesEpsilon(const std::string& symbol);

};

#endif // FIRST_FOLLOW_H
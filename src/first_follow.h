#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"
#include <map>
#include <set>

class FirstFollow {
public:
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;

    void computeFirstSets(const Grammar& grammar);
    void computeFollowSets(const Grammar& grammar);
    void printFirstSets() const;
    void printFollowSets() const;
};

#endif
#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>

const std::string EPSILON = "epsilon"; 

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
};

class Grammar {
public:
    std::string startSymbol;
    std::vector<Production> productions;
    std::set<std::string> nonTerminals;
    std::set<std::string> terminals;

    bool loadFromFile(const std::string& filename);
    void printGrammar() const;
    void applyLeftFactoring();
    void removeLeftRecursion();
    
    bool isNonTerminal(const std::string& symbol) const;
    bool isTerminal(const std::string& symbol) const;

private:
    void categorizeSymbol(const std::string& symbol);
    std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif
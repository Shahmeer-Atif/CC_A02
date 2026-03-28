#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>

// Standardize how you represent epsilon to avoid hardcoding strings everywhere
const std::string EPSILON = "epsilon"; 

// A single production rule: LHS -> RHS
struct Production {
    std::string lhs;                  // e.g., "Expr"
    std::vector<std::string> rhs;     // e.g., ["Expr", "+", "Term"]

    // Quick helper to print a single rule cleanly
    void print() const {
        std::cout << lhs << " -> ";
        if (rhs.empty()) {
            std::cout << EPSILON;
        } else {
            for (size_t i = 0; i < rhs.size(); ++i) {
                std::cout << rhs[i] << (i < rhs.size() - 1 ? " " : "");
            }
        }
        std::cout << std::endl;
    }
};

class Grammar {
public:
    std::string startSymbol;
    std::vector<Production> productions;
    std::set<std::string> nonTerminals;
    std::set<std::string> terminals;

    // Core functionality
    bool loadFromFile(const std::string& filename);
    void printGrammar() const;

    // Helper methods
    bool isNonTerminal(const std::string& symbol) const;
    bool isTerminal(const std::string& symbol) const;
    void applyLeftFactoring();
    void removeLeftRecursion();

private:
    // Helper to extract terminals vs non-terminals automatically
    void categorizeSymbol(const std::string& symbol);
    
    // String splitting utility (since C++ doesn't have one built-in)
    std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif // GRAMMAR_H
#include "first_follow.h"
#include <iomanip>

void FirstFollow::computeFirstSets(const Grammar& grammar) {
    bool changed = true;
    for (const std::string& t : grammar.terminals) firstSets[t].insert(t);

    while (changed) {
        changed = false;
        for (const Production& prod : grammar.productions) {
            std::string lhs = prod.lhs;
            size_t originalSize = firstSets[lhs].size();

            if (prod.rhs.empty() || (prod.rhs.size() == 1 && prod.rhs[0] == EPSILON)) {
                firstSets[lhs].insert(EPSILON);
            } else {
                bool allDeriveEpsilon = true;
                for (const std::string& sym : prod.rhs) {
                    for (const std::string& f : firstSets[sym]) if (f != EPSILON) firstSets[lhs].insert(f);
                    if (firstSets[sym].find(EPSILON) == firstSets[sym].end()) { allDeriveEpsilon = false; break; }
                }
                if (allDeriveEpsilon) firstSets[lhs].insert(EPSILON);
            }
            if (firstSets[lhs].size() > originalSize) changed = true;
        }
    }
}

void FirstFollow::computeFollowSets(const Grammar& grammar) {
    bool changed = true;
    followSets[grammar.startSymbol].insert("$");

    while (changed) {
        changed = false;
        for (const Production& prod : grammar.productions) {
            std::string lhs = prod.lhs;
            for (size_t i = 0; i < prod.rhs.size(); ++i) {
                std::string currentSymbol = prod.rhs[i];
                if (grammar.isTerminal(currentSymbol) || currentSymbol == EPSILON) continue;

                size_t originalSize = followSets[currentSymbol].size();
                bool betaDerivesEpsilon = true;

                for (size_t j = i + 1; j < prod.rhs.size(); ++j) {
                    std::string nextSymbol = prod.rhs[j];
                    for (const std::string& f : firstSets[nextSymbol]) if (f != EPSILON) followSets[currentSymbol].insert(f);
                    if (firstSets[nextSymbol].find(EPSILON) == firstSets[nextSymbol].end()) { betaDerivesEpsilon = false; break; }
                }

                if (betaDerivesEpsilon) {
                    for (const std::string& f : followSets[lhs]) followSets[currentSymbol].insert(f);
                }
                if (followSets[currentSymbol].size() > originalSize) changed = true;
            }
        }
    }
}

void FirstFollow::printFirstSets() const {
    std::cout << "\n--- FIRST Sets ---\n" << std::left << std::setw(15) << "Non-Terminal" << "FIRST Set\n" << std::string(40, '-') << "\n";
    for (const auto& pair : firstSets) {
        if (!pair.first.empty() && std::isupper(pair.first[0])) {
            std::cout << std::left << std::setw(15) << pair.first << "{ ";
            auto it = pair.second.begin();
            while (it != pair.second.end()) { std::cout << *it; ++it; if (it != pair.second.end()) std::cout << ", "; }
            std::cout << " }\n";
        }
    }
}

void FirstFollow::printFollowSets() const {
    std::cout << "\n--- FOLLOW Sets ---\n" << std::left << std::setw(15) << "Non-Terminal" << "FOLLOW Set\n" << std::string(40, '-') << "\n";
    for (const auto& pair : followSets) {
        if (!pair.first.empty() && std::isupper(pair.first[0])) {
            std::cout << std::left << std::setw(15) << pair.first << "{ ";
            auto it = pair.second.begin();
            while (it != pair.second.end()) { std::cout << *it; ++it; if (it != pair.second.end()) std::cout << ", "; }
            std::cout << " }\n";
        }
    }
}
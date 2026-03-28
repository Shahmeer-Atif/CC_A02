#include "first_follow.h"
#include <iostream>
#include <iomanip>

void FirstFollow::computeFirstSets(const Grammar& grammar) {
    bool changed = true;

    // Rule 1: If X is a terminal, FIRST(X) = {X}
    for (const std::string& t : grammar.terminals) {
        firstSets[t].insert(t);
    }

    // Loop until no new symbols are added to ANY set (Fixed-Point iteration)
    while (changed) {
        changed = false;

        // Go through every single production rule
        for (const Production& prod : grammar.productions) {
            std::string lhs = prod.lhs;
            size_t originalSize = firstSets[lhs].size();

            // Rule 2: If X -> epsilon, add epsilon to FIRST(X)
            if (prod.rhs.empty() || (prod.rhs.size() == 1 && prod.rhs[0] == EPSILON)) {
                firstSets[lhs].insert(EPSILON);
            } 
            // Rule 3: If X -> Y1Y2...Yn
            else {
                bool allDeriveEpsilon = true;

                for (const std::string& sym : prod.rhs) {
                    // Add everything from FIRST(Yi) to FIRST(X), except epsilon
                    for (const std::string& f : firstSets[sym]) {
                        if (f != EPSILON) {
                            firstSets[lhs].insert(f);
                        }
                    }

                    // If this symbol DOES NOT have epsilon in its FIRST set,
                    // we stop checking the rest of the symbols in this rule.
                    if (firstSets[sym].find(EPSILON) == firstSets[sym].end()) {
                        allDeriveEpsilon = false;
                        break;
                    }
                }

                // If we made it through all symbols and they ALL can be epsilon,
                // then FIRST(X) must also contain epsilon.
                if (allDeriveEpsilon) {
                    firstSets[lhs].insert(EPSILON);
                }
            }

            // Did we add anything new to FIRST(lhs)? If yes, keep the while loop going.
            if (firstSets[lhs].size() > originalSize) {
                changed = true;
            }
        }
    }
}

void FirstFollow::printFirstSets() const {
    std::cout << "\n--- FIRST Sets ---\n";
    // Formatting for a nice table look
    std::cout << std::left << std::setw(15) << "Non-Terminal" << "FIRST Set\n";
    std::cout << std::string(40, '-') << "\n";

    for (const auto& pair : firstSets) {
        // We only want to print FIRST sets for Non-Terminals
        // Terminals are in the map too, but we don't need to display them
        if (!pair.first.empty() && std::isupper(pair.first[0])) {
            std::cout << std::left << std::setw(15) << pair.first << "{ ";
            
            auto it = pair.second.begin();
            while (it != pair.second.end()) {
                std::cout << *it;
                ++it;
                if (it != pair.second.end()) std::cout << ", ";
            }
            std::cout << " }\n";
        }
    }
    std::cout << std::string(40, '-') << "\n";
}

void FirstFollow::computeFollowSets(const Grammar& grammar) {
    bool changed = true;

    // Rule 1: Place $ in FOLLOW(S) where S is start symbol
    followSets[grammar.startSymbol].insert("$");

    // Loop until no new symbols are added (Fixed-Point iteration)
    while (changed) {
        changed = false;

        for (const Production& prod : grammar.productions) {
            std::string lhs = prod.lhs;

            // Iterate through every symbol on the right-hand side
            for (size_t i = 0; i < prod.rhs.size(); ++i) {
                std::string currentSymbol = prod.rhs[i];

                // We only calculate FOLLOW sets for Non-Terminals
                if (grammar.isTerminal(currentSymbol) || currentSymbol == EPSILON) {
                    continue;
                }

                size_t originalSize = followSets[currentSymbol].size();
                bool betaDerivesEpsilon = true;

                // Look at everything that comes AFTER currentSymbol (this is "beta")
                for (size_t j = i + 1; j < prod.rhs.size(); ++j) {
                    std::string nextSymbol = prod.rhs[j];

                    // Rule 2: Add FIRST(beta) - {epsilon} to FOLLOW(currentSymbol)
                    for (const std::string& f : firstSets[nextSymbol]) {
                        if (f != EPSILON) {
                            followSets[currentSymbol].insert(f);
                        }
                    }

                    // If nextSymbol CANNOT derive epsilon, we stop looking further ahead
                    if (firstSets[nextSymbol].find(EPSILON) == firstSets[nextSymbol].end()) {
                        betaDerivesEpsilon = false;
                        break;
                    }
                }

                // Rule 3: If beta can derive epsilon (or if currentSymbol is at the end of the rule)
                if (betaDerivesEpsilon) {
                    for (const std::string& f : followSets[lhs]) {
                        followSets[currentSymbol].insert(f);
                    }
                }

                // Did we add anything new?
                if (followSets[currentSymbol].size() > originalSize) {
                    changed = true;
                }
            }
        }
    }
}

void FirstFollow::printFollowSets() const {
    std::cout << "\n--- FOLLOW Sets ---\n";
    std::cout << std::left << std::setw(15) << "Non-Terminal" << "FOLLOW Set\n";
    std::cout << std::string(40, '-') << "\n";

    for (const auto& pair : followSets) {
        // Only print for Non-Terminals
        if (!pair.first.empty() && std::isupper(pair.first[0])) {
            std::cout << std::left << std::setw(15) << pair.first << "{ ";
            
            auto it = pair.second.begin();
            while (it != pair.second.end()) {
                std::cout << *it;
                ++it;
                if (it != pair.second.end()) std::cout << ", ";
            }
            std::cout << " }\n";
        }
    }
    std::cout << std::string(40, '-') << "\n";
}
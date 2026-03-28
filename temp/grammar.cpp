#include "grammar.h"
#include <algorithm>
#include <cctype>
#include <map> // Make sure this is at the top of grammar.cpp

// --- Helper Functions for String Manipulation ---

// Trims leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Splits a string by a delimiter
std::vector<std::string> Grammar::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        std::string trimmed = trim(token);
        if (!trimmed.empty()) {
            tokens.push_back(trimmed);
        }
    }
    return tokens;
}

// --- Core Grammar Logic ---

void Grammar::categorizeSymbol(const std::string& symbol) {
    if (symbol == "epsilon" || symbol == "@") {
        // Epsilon is special, we usually don't put it in the normal terminal set
        // to avoid putting it in the parsing table columns.
        return; 
    }
    
    // Assignment rule: Non-terminals start with an Uppercase letter.
    if (!symbol.empty() && std::isupper(symbol[0])) {
        nonTerminals.insert(symbol);
    } else {
        terminals.insert(symbol);
    }
}

bool Grammar::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    bool isFirstLine = true;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // Find the "->" separator
        size_t arrowPos = line.find("->");
        if (arrowPos == std::string::npos) {
            std::cerr << "Warning: Skipping invalid line (no '->'): " << line << std::endl;
            continue;
        }

        // Extract Left Hand Side (LHS)
        std::string lhs = trim(line.substr(0, arrowPos));
        categorizeSymbol(lhs);
        
        // The very first non-terminal we see is the Start Symbol
        if (isFirstLine) {
            startSymbol = lhs;
            isFirstLine = false;
        }

        // Extract Right Hand Side (RHS) and split by '|'
        std::string rhsPart = line.substr(arrowPos + 2);
        std::vector<std::string> alternatives = split(rhsPart, '|');

        // Process each alternative
        for (const std::string& alt : alternatives) {
            Production prod;
            prod.lhs = lhs;

            // Split the alternative by spaces to get individual symbols
            std::vector<std::string> symbols = split(alt, ' ');
            
            for (std::string& sym : symbols) {
                // Standardize epsilon representation
                if (sym == "@") sym = EPSILON; 
                
                prod.rhs.push_back(sym);
                categorizeSymbol(sym);
            }
            productions.push_back(prod);
        }
    }

    file.close();
    return true;
}

void Grammar::printGrammar() const {
    std::cout << "--- Grammar ---" << std::endl;
    std::cout << "Start Symbol: " << startSymbol << std::endl;
    std::cout << "Productions:" << std::endl;
    for (size_t i = 0; i < productions.size(); ++i) {
        std::cout << i + 1 << ". ";
        
        std::cout << productions[i].lhs << " -> ";
        if (productions[i].rhs.empty() || (productions[i].rhs.size() == 1 && productions[i].rhs[0] == EPSILON)) {
            std::cout << "epsilon";
        } else {
            for (size_t j = 0; j < productions[i].rhs.size(); ++j) {
                std::cout << productions[i].rhs[j] << (j < productions[i].rhs.size() - 1 ? " " : "");
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nNon-Terminals: ";
    for (const auto& nt : nonTerminals) std::cout << nt << " ";
    
    std::cout << "\nTerminals: ";
    for (const auto& t : terminals) std::cout << t << " ";
    std::cout << "\n-----------------\n";
}

bool Grammar::isNonTerminal(const std::string& symbol) const {
    return nonTerminals.find(symbol) != nonTerminals.end();
}

bool Grammar::isTerminal(const std::string& symbol) const {
    return terminals.find(symbol) != terminals.end();
}


void Grammar::applyLeftFactoring() {
    bool changed = true;
    
    // We keep looping until no more common prefixes are found anywhere
    while (changed) {
        changed = false;
        
        // Step 1: Group productions by their Left-Hand Side (LHS)
        std::map<std::string, std::vector<std::vector<std::string>>> groupedRules;
        for (const auto& prod : productions) {
            groupedRules[prod.lhs].push_back(prod.rhs);
        }
        
        // Clear the old productions list, we will rebuild it dynamically
        productions.clear();
        
        // Step 2: Check each group (e.g., all rules for "Expr")
        for (auto& pair : groupedRules) {
            std::string lhs = pair.first;
            std::vector<std::vector<std::string>>& rhsList = pair.second;
            
            bool factoredInThisNt = false; // Flag to track if we changed this specific Non-Terminal
            
            // Step 3: Compare every pair of RHSs to find a common prefix
            for (size_t i = 0; i < rhsList.size() && !factoredInThisNt; ++i) {
                for (size_t j = i + 1; j < rhsList.size() && !factoredInThisNt; ++j) {
                    
                    std::vector<std::string> prefix;
                    size_t k = 0;
                    
                    // Find the overlapping symbols
                    while (k < rhsList[i].size() && k < rhsList[j].size() && 
                           rhsList[i][k] == rhsList[j][k]) {
                        prefix.push_back(rhsList[i][k]);
                        k++;
                    }
                    
                    // If we found at least 1 common symbol, we have a prefix to factor!
                    if (!prefix.empty()) {
                        factoredInThisNt = true;
                        changed = true; // Tell the outer loop to run again later
                        
                        std::vector<std::vector<std::string>> rulesWithPrefix;
                        std::vector<std::vector<std::string>> rulesWithoutPrefix;
                        
                        // Separate rules that have this prefix from rules that don't
                        for (const auto& rhs : rhsList) {
                            bool hasPrefix = true;
                            if (rhs.size() < prefix.size()) {
                                hasPrefix = false;
                            } else {
                                for (size_t p = 0; p < prefix.size(); ++p) {
                                    if (rhs[p] != prefix[p]) { hasPrefix = false; break; }
                                }
                            }
                            
                            if (hasPrefix) rulesWithPrefix.push_back(rhs);
                            else rulesWithoutPrefix.push_back(rhs);
                        }
                        
                        // Step 4: Create the new Non-Terminal (e.g., "Stmt'")
                        std::string newLhs = lhs + "'";
                        nonTerminals.insert(newLhs);
                        
                        // Rule Type 1: A -> prefix A'
                        Production baseRule;
                        baseRule.lhs = lhs;
                        baseRule.rhs = prefix;
                        baseRule.rhs.push_back(newLhs);
                        productions.push_back(baseRule);
                        
                        // Rule Type 2: A' -> leftovers
                        for (const auto& rhs : rulesWithPrefix) {
                            Production leftoverRule;
                            leftoverRule.lhs = newLhs;
                            
                            // If the original rule was exactly the prefix, leftover is epsilon
                            if (rhs.size() == prefix.size()) {
                                leftoverRule.rhs.push_back(EPSILON);
                            } else {
                                // Otherwise, copy the remaining symbols
                                for (size_t p = prefix.size(); p < rhs.size(); ++p) {
                                    leftoverRule.rhs.push_back(rhs[p]);
                                }
                            }
                            productions.push_back(leftoverRule);
                        }
                        
                        // Rule Type 3: Put the untouched rules back exactly as they were
                        for (const auto& rhs : rulesWithoutPrefix) {
                            Production untouchedRule;
                            untouchedRule.lhs = lhs;
                            untouchedRule.rhs = rhs;
                            productions.push_back(untouchedRule);
                        }
                    }
                }
            }
            
            // If we checked all pairs for this Non-Terminal and found NO prefixes, 
            // just put its rules back into the main list unharmed.
            if (!factoredInThisNt) {
                for (const auto& rhs : rhsList) {
                    Production originalRule;
                    originalRule.lhs = lhs;
                    originalRule.rhs = rhs;
                    productions.push_back(originalRule);
                }
            }
        }
    }
}
void Grammar::removeLeftRecursion() {
    // We need a fixed, ordered list of non-terminals (A1, A2, ..., An)
    std::vector<std::string> ntList(nonTerminals.begin(), nonTerminals.end());
    
    // Create a new list to hold our transformed productions
    std::vector<Production> updatedProductions;

    // Loop i from 1 to n (using 0-based indexing)
    for (size_t i = 0; i < ntList.size(); ++i) {
        std::string Ai = ntList[i];

        // ---------------------------------------------------------
        // PART 1: Eliminate Indirect Left Recursion
        // ---------------------------------------------------------
        // Loop j from 1 to i-1
        for (size_t j = 0; j < i; ++j) {
            std::string Aj = ntList[j];
            std::vector<Production> newAiProductions;

            // Look at all current rules for Ai
            for (auto it = productions.begin(); it != productions.end(); ++it) {
                if (it->lhs == Ai) {
                    // Check if Ai -> Aj gamma (Indirect recursion detected!)
                    if (!it->rhs.empty() && it->rhs[0] == Aj) {
                        std::vector<std::string> gamma(it->rhs.begin() + 1, it->rhs.end());

                        // Substitute with all current rules of Aj
                        for (const auto& prodAj : productions) {
                            if (prodAj.lhs == Aj) {
                                Production subRule;
                                subRule.lhs = Ai;
                                
                                // Add Aj's RHS (delta)
                                if (!(prodAj.rhs.size() == 1 && prodAj.rhs[0] == EPSILON)) {
                                    subRule.rhs = prodAj.rhs;
                                }
                                // Append gamma
                                subRule.rhs.insert(subRule.rhs.end(), gamma.begin(), gamma.end());
                                
                                if (subRule.rhs.empty()) subRule.rhs.push_back(EPSILON);
                                newAiProductions.push_back(subRule);
                            }
                        }
                    } else {
                        // Not an indirect recursive rule, keep as is
                        newAiProductions.push_back(*it);
                    }
                }
            }

            // Replace Ai's old rules with the newly substituted rules
            // We use an iterator to remove the old ones and add the new ones
            for (auto it = productions.begin(); it != productions.end(); ) {
                if (it->lhs == Ai) {
                    it = productions.erase(it);
                } else {
                    ++it;
                }
            }
            productions.insert(productions.end(), newAiProductions.begin(), newAiProductions.end());
        }

        // ---------------------------------------------------------
        // PART 2: Eliminate Direct Left Recursion
        // ---------------------------------------------------------
        std::vector<std::vector<std::string>> alphas; // Recursive parts (A -> A alpha)
        std::vector<std::vector<std::string>> betas;  // Non-recursive parts (A -> beta)

        // Separate alphas and betas for Ai
        for (const auto& prod : productions) {
            if (prod.lhs == Ai) {
                if (!prod.rhs.empty() && prod.rhs[0] == Ai) {
                    // It is direct left recursive! Grab the 'alpha' part
                    alphas.push_back(std::vector<std::string>(prod.rhs.begin() + 1, prod.rhs.end()));
                } else {
                    // It is a 'beta' part
                    betas.push_back(prod.rhs);
                }
            }
        }

        // If alphas is empty, there is no direct left recursion for this non-terminal
        if (!alphas.empty()) {
            std::string newAi = Ai + "Prime"; // Using "Prime" as requested in assignment example
            nonTerminals.insert(newAi);

            // 1. Create A -> beta A'
            for (const auto& beta : betas) {
                Production p;
                p.lhs = Ai;
                if (beta.size() == 1 && beta[0] == EPSILON) {
                    p.rhs.push_back(newAi); // If beta was epsilon, just A -> A'
                } else {
                    p.rhs = beta;
                    p.rhs.push_back(newAi);
                }
                updatedProductions.push_back(p);
            }

            // 2. Create A' -> alpha A'
            for (const auto& alpha : alphas) {
                Production p;
                p.lhs = newAi;
                p.rhs = alpha;
                p.rhs.push_back(newAi);
                updatedProductions.push_back(p);
            }

            // 3. Create A' -> epsilon
            Production pEps;
            pEps.lhs = newAi;
            pEps.rhs.push_back(EPSILON);
            updatedProductions.push_back(pEps);

        } else {
            // No direct left recursion, just keep the rules exactly as they were
            for (const auto& prod : productions) {
                if (prod.lhs == Ai) {
                    updatedProductions.push_back(prod);
                }
            }
        }
    }

    // Finally, overwrite the grammar's production list with the fully resolved list
    productions = updatedProductions;
}
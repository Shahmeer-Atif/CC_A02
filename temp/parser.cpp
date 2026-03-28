#include "parser.h"
#include <iostream>
#include <iomanip>
#include "stack.h" // Add this to the top of parser.cpp!
#include <sstream>


// Helper: Computes the FIRST set for a specific sequence of symbols (the RHS of a rule)
std::set<std::string> Parser::getFirstOfSequence(const std::vector<std::string>& rhs, const FirstFollow& ff, const Grammar& grammar) {
    std::set<std::string> result;
    
    // If the rule is A -> epsilon
    if (rhs.empty() || (rhs.size() == 1 && rhs[0] == EPSILON)) {
        result.insert(EPSILON);
        return result;
    }

    bool allDeriveEpsilon = true;
    for (const std::string& sym : rhs) {
        std::set<std::string> symFirst;
        
        if (grammar.isTerminal(sym)) {
            symFirst.insert(sym);
        } else {
            // Find it in our pre-calculated FIRST sets
            auto it = ff.firstSets.find(sym);
            if (it != ff.firstSets.end()) {
                symFirst = it->second;
            }
        }

        // Add everything except epsilon to our result
        for (const std::string& f : symFirst) {
            if (f != EPSILON) result.insert(f);
        }

        // If this symbol CANNOT derive epsilon, stop looking ahead
        if (symFirst.find(EPSILON) == symFirst.end()) {
            allDeriveEpsilon = false;
            break;
        }
    }

    if (allDeriveEpsilon) {
        result.insert(EPSILON);
    }
    
    return result;
}

void Parser::buildParsingTable(const Grammar& grammar, const FirstFollow& ff) {
    isLL1Grammar = true;

    for (const Production& prod : grammar.productions) {
        std::string A = prod.lhs;
        
        // Find FIRST(alpha)
        std::set<std::string> firstOfRhs = getFirstOfSequence(prod.rhs, ff, grammar);

        // 1. For each terminal 'a' in FIRST(alpha), add A -> alpha to M[A, a]
        for (const std::string& a : firstOfRhs) {
            if (a != EPSILON) {
                // Check for LL(1) conflict (is there already a rule in this cell?)
                if (parsingTable[A].find(a) != parsingTable[A].end()) {
                    isLL1Grammar = false;
                    std::cerr << "LL(1) Conflict Detected at M[" << A << ", " << a << "]\n";
                }
                parsingTable[A][a] = prod;
            }
        }

        // 2. If epsilon is in FIRST(alpha), add A -> alpha to M[A, b] for each 'b' in FOLLOW(A)
        if (firstOfRhs.find(EPSILON) != firstOfRhs.end()) {
            // Get FOLLOW(A)
            auto it = ff.followSets.find(A);
            if (it != ff.followSets.end()) {
                for (const std::string& b : it->second) {
                    // Check for LL(1) conflict
                    if (parsingTable[A].find(b) != parsingTable[A].end() && 
                        parsingTable[A][b].rhs != prod.rhs) { 
                        isLL1Grammar = false;
                        std::cerr << "LL(1) Conflict Detected at M[" << A << ", " << b << "]\n";
                    }
                    parsingTable[A][b] = prod;
                }
            }
        }
    }
}

void Parser::printParsingTable(const Grammar& grammar) const {
    std::cout << "\n--- LL(1) Parsing Table ---\n";
    
    if (isLL1Grammar) {
        std::cout << "Status: Grammar IS LL(1).\n\n";
    } else {
        std::cout << "Status: Grammar is NOT LL(1) (Contains conflicts).\n\n";
    }

    // Print Header Row (Terminals + $)
    std::cout << std::left << std::setw(15) << "Non-Terminal";
    for (const std::string& t : grammar.terminals) {
        std::cout << std::left << std::setw(25) << t;
    }
    std::cout << std::left << std::setw(25) << "$" << "\n";
    
    std::cout << std::string(15 + (grammar.terminals.size() + 1) * 25, '-') << "\n";

    // Print each row (Non-Terminals)
    for (const std::string& nt : grammar.nonTerminals) {
        std::cout << std::left << std::setw(15) << nt;
        
        // Columns for Terminals
        for (const std::string& t : grammar.terminals) {
            auto cell = parsingTable.at(nt).find(t);
            if (cell != parsingTable.at(nt).end()) {
                // Construct the rule string to display nicely
                std::string ruleStr = cell->second.lhs + " -> ";
                if (cell->second.rhs.empty() || (cell->second.rhs.size() == 1 && cell->second.rhs[0] == EPSILON)) {
                    ruleStr += "epsilon";
                } else {
                    for (size_t i = 0; i < cell->second.rhs.size(); ++i) {
                        ruleStr += cell->second.rhs[i] + (i < cell->second.rhs.size() - 1 ? " " : "");
                    }
                }
                std::cout << std::left << std::setw(25) << ruleStr;
            } else {
                std::cout << std::left << std::setw(25) << "-"; // Empty cell
            }
        }

        // Column for $
        auto cell = parsingTable.at(nt).find("$");
        if (cell != parsingTable.at(nt).end()) {
             std::string ruleStr = cell->second.lhs + " -> ";
             if (cell->second.rhs.empty() || (cell->second.rhs.size() == 1 && cell->second.rhs[0] == EPSILON)) {
                 ruleStr += "epsilon";
             } else {
                 for (size_t i = 0; i < cell->second.rhs.size(); ++i) {
                     ruleStr += cell->second.rhs[i] + (i < cell->second.rhs.size() - 1 ? " " : "");
                 }
             }
             std::cout << std::left << std::setw(25) << ruleStr;
        } else {
             std::cout << std::left << std::setw(25) << "-";
        }
        std::cout << "\n";
    }
    std::cout << std::string(15 + (grammar.terminals.size() + 1) * 25, '-') << "\n";
}

void Parser::parseString(const std::string& inputLine, const Grammar& grammar, const FirstFollow& ff) {
    std::cout << "\n=== Parsing Trace for: " << inputLine << " ===\n";
    
    std::vector<std::string> inputTokens;
    std::stringstream ss(inputLine);
    std::string token;
    while (ss >> token) {
        inputTokens.push_back(token);
    }
    if (inputTokens.empty() || inputTokens.back() != "$") {
        inputTokens.push_back("$"); 
    }

    Stack stack;
    stack.push("$");
    stack.push(grammar.startSymbol);

    size_t inputPtr = 0;
    int step = 1;
    int errorCount = 0; // Track errors!

    std::cout << std::left << std::setw(6) << "Step" 
              << std::setw(35) << "Stack" 
              << std::setw(20) << "Input" 
              << "Action\n";
    std::cout << std::string(85, '-') << "\n";

    while (!stack.isEmpty() && inputPtr < inputTokens.size()) {
        std::string X = stack.top();
        std::string a = inputTokens[inputPtr];

        std::string remainingInput = "";
        for (size_t i = inputPtr; i < inputTokens.size(); ++i) {
            remainingInput += inputTokens[i] + " ";
        }

        std::cout << std::left << std::setw(6) << step++ 
                  << std::setw(35) << stack.getStackString() 
                  << std::setw(20) << remainingInput;

        // Case 1: Parsing successful
        if (X == "$" && a == "$") {
            std::cout << "Accept\n";
            if (errorCount == 0) {
                std::cout << "\nResult: String accepted successfully!\n";
            } else {
                std::cout << "\nResult: Parsing completed with " << errorCount << " error(s).\n";
            }
            return;
        }

        // Case 2: Match terminal
        if (X == a) {
            std::cout << "Match " << a << "\n";
            stack.pop();
            inputPtr++; 
        }
        // Case 3: X is a Non-Terminal
        else if (grammar.isNonTerminal(X)) {
            if (parsingTable.find(X) != parsingTable.end() && 
                parsingTable[X].find(a) != parsingTable[X].end()) {
                
                Production prod = parsingTable[X][a];
                
                std::cout << prod.lhs << " -> ";
                if (prod.rhs.empty() || (prod.rhs.size() == 1 && prod.rhs[0] == EPSILON)) {
                    std::cout << "epsilon\n";
                    stack.pop(); 
                } else {
                    for (const auto& sym : prod.rhs) std::cout << sym << " ";
                    std::cout << "\n";
                    
                    stack.pop();
                    for (int i = prod.rhs.size() - 1; i >= 0; --i) {
                        stack.push(prod.rhs[i]);
                    }
                }
            } else {
                // --- ERROR HANDLING: EMPTY TABLE ENTRY ---
                errorCount++;
                std::cout << "ERROR: Unexpected '" << a << "'\n";
                
                // Show what was expected (all terminals that have entries for X)
                std::cout << std::string(61, ' ') << "Expected: ";
                bool first = true;
                for (const auto& pair : parsingTable[X]) {
                    if (!first) std::cout << " or ";
                    std::cout << pair.first;
                    first = false;
                }
                std::cout << "\n";

                // PANIC MODE RECOVERY: 
                // Is the unexpected token 'a' in the FOLLOW set of X?
                bool inFollow = false;
                auto followIt = ff.followSets.find(X);
                if (followIt != ff.followSets.end()) {
                    if (followIt->second.find(a) != followIt->second.end()) {
                        inFollow = true;
                    }
                }

                if (inFollow) {
                    // If 'a' is in FOLLOW(X), assume X was missing entirely. Pop X to sync.
                    std::cout << std::string(61, ' ') << "Popping '" << X << "' to synchronize.\n";
                    stack.pop();
                } else {
                    // Otherwise, 'a' is a rogue token. Skip it.
                    std::cout << std::string(61, ' ') << "Skipping '" << a << "'\n";
                    inputPtr++;
                }
            }
        }
        // Case 4: X is a terminal but doesn't match 'a'
        else {
            errorCount++;
            std::cout << "ERROR: Expected '" << X << "' but found '" << a << "'\n";
            std::cout << std::string(61, ' ') << "Popping expected '" << X << "'\n";
            stack.pop(); // Pop the expected terminal to keep trying
        }
    }
    
    // If we break the loop prematurely
    if (errorCount > 0) {
        std::cout << "\nResult: Parsing completed with " << errorCount << " error(s).\n";
    } else {
        std::cout << "\nResult: Parsing failed prematurely.\n";
    }
}
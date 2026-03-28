#include "parser.h"
#include "error_handler.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::set<std::string> Parser::getFirstOfSequence(const std::vector<std::string>& rhs, const FirstFollow& ff, const Grammar& grammar) {
    std::set<std::string> result;
    if (rhs.empty() || (rhs.size() == 1 && rhs[0] == EPSILON)) { result.insert(EPSILON); return result; }

    bool allDeriveEpsilon = true;
    for (const std::string& sym : rhs) {
        std::set<std::string> symFirst;
        if (grammar.isTerminal(sym)) symFirst.insert(sym);
        else {
            auto it = ff.firstSets.find(sym);
            if (it != ff.firstSets.end()) symFirst = it->second;
        }

        for (const std::string& f : symFirst) if (f != EPSILON) result.insert(f);
        if (symFirst.find(EPSILON) == symFirst.end()) { allDeriveEpsilon = false; break; }
    }
    if (allDeriveEpsilon) result.insert(EPSILON);
    return result;
}

void Parser::buildParsingTable(const Grammar& grammar, const FirstFollow& ff) {
    isLL1Grammar = true;
    for (const Production& prod : grammar.productions) {
        std::string A = prod.lhs;
        std::set<std::string> firstOfRhs = getFirstOfSequence(prod.rhs, ff, grammar);

        for (const std::string& a : firstOfRhs) {
            if (a != EPSILON) parsingTable[A][a] = prod;
        }
        if (firstOfRhs.find(EPSILON) != firstOfRhs.end()) {
            auto it = ff.followSets.find(A);
            if (it != ff.followSets.end()) {
                for (const std::string& b : it->second) parsingTable[A][b] = prod;
            }
        }
    }
}

void Parser::printParsingTable(const Grammar& grammar) const {
    std::cout << "\n--- LL(1) Parsing Table ---\n";
    std::cout << std::left << std::setw(15) << "Non-Term";
    for (const std::string& t : grammar.terminals) std::cout << std::left << std::setw(20) << t;
    std::cout << std::left << std::setw(20) << "$" << "\n" << std::string(80, '-') << "\n";

    for (const std::string& nt : grammar.nonTerminals) {
        std::cout << std::left << std::setw(15) << nt;
        for (const std::string& t : grammar.terminals) {
            if (parsingTable.at(nt).find(t) != parsingTable.at(nt).end()) std::cout << std::left << std::setw(20) << "Rule";
            else std::cout << std::left << std::setw(20) << "-";
        }
        if (parsingTable.at(nt).find("$") != parsingTable.at(nt).end()) std::cout << std::left << std::setw(20) << "Rule";
        else std::cout << std::left << std::setw(20) << "-";
        std::cout << "\n";
    }
}

TreeNode* Parser::parseString(const std::string& inputLine, const Grammar& grammar, const FirstFollow& ff) {
    std::cout << "\n=== Parsing Trace for: " << inputLine << " ===\n";
    std::vector<std::string> inputTokens;
    std::stringstream ss(inputLine);
    std::string token;
    while (ss >> token) inputTokens.push_back(token);
    if (inputTokens.empty() || inputTokens.back() != "$") inputTokens.push_back("$");

    TreeNode* root = new TreeNode(grammar.startSymbol);
    Stack stack;
    stack.push(new TreeNode("$"));
    stack.push(root);

    size_t inputPtr = 0; int step = 1; int errorCount = 0;
    std::cout << std::left << std::setw(6) << "Step" << std::setw(35) << "Stack" << std::setw(20) << "Input" << "Action\n";
    std::cout << std::string(85, '-') << "\n";

    while (!stack.isEmpty() && inputPtr < inputTokens.size()) {
        TreeNode* currentNode = stack.top();
        std::string X = currentNode->symbol;
        std::string a = inputTokens[inputPtr];

        std::string remainingInput = "";
        for (size_t i = inputPtr; i < inputTokens.size(); ++i) remainingInput += inputTokens[i] + " ";

        std::cout << std::left << std::setw(6) << step++ << std::setw(35) << stack.getStackString() << std::setw(20) << remainingInput;

        if (X == "$" && a == "$") {
            std::cout << "Accept\n\nResult: Parsed with " << errorCount << " errors.\n";
            return root;
        }

        if (X == a) {
            std::cout << "Match " << a << "\n";
            stack.pop(); inputPtr++;
        } else if (grammar.isNonTerminal(X)) {
            if (parsingTable.find(X) != parsingTable.end() && parsingTable[X].find(a) != parsingTable[X].end()) {
                Production prod = parsingTable[X][a];
                std::cout << prod.lhs << " -> ";
                if (prod.rhs.empty() || (prod.rhs.size() == 1 && prod.rhs[0] == EPSILON)) {
                    std::cout << "epsilon\n";
                    currentNode->children.push_back(new TreeNode("epsilon"));
                    stack.pop();
                } else {
                    for (const auto& sym : prod.rhs) std::cout << sym << " ";
                    std::cout << "\n";
                    stack.pop();
                    std::vector<TreeNode*> newChildren;
                    for (size_t i = 0; i < prod.rhs.size(); ++i) {
                        TreeNode* child = new TreeNode(prod.rhs[i]);
                        currentNode->children.push_back(child);
                        newChildren.push_back(child);
                    }
                    for (int i = newChildren.size() - 1; i >= 0; --i) stack.push(newChildren[i]);
                }
            } else {
                errorCount++;
                std::cout << "ERROR: Unexpected '" << a << "'\n";
                ErrorHandler::handlePanicMode(a, X, ff, stack, inputPtr);
            }
        } else {
            errorCount++;
            std::cout << "ERROR: Expected '" << X << "' found '" << a << "'\n";
            stack.pop();
        }
    }
    return root;
}
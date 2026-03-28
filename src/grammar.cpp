#include "grammar.h"
#include <algorithm>
#include <cctype>
#include <map>

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> Grammar::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        std::string trimmed = trim(token);
        if (!trimmed.empty()) tokens.push_back(trimmed);
    }
    return tokens;
}

void Grammar::categorizeSymbol(const std::string& symbol) {
    if (symbol == "epsilon" || symbol == "@") return; 
    if (!symbol.empty() && std::isupper(symbol[0])) {
        nonTerminals.insert(symbol);
    } else {
        terminals.insert(symbol);
    }
}

bool Grammar::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    bool isFirstLine = true;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        size_t arrowPos = line.find("->");
        if (arrowPos == std::string::npos) continue;

        std::string lhs = trim(line.substr(0, arrowPos));
        categorizeSymbol(lhs);
        
        if (isFirstLine) {
            startSymbol = lhs;
            isFirstLine = false;
        }

        std::string rhsPart = line.substr(arrowPos + 2);
        std::vector<std::string> alternatives = split(rhsPart, '|');

        for (const std::string& alt : alternatives) {
            Production prod;
            prod.lhs = lhs;
            std::vector<std::string> symbols = split(alt, ' ');
            for (std::string& sym : symbols) {
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
    std::cout << "\n--- Transformed Grammar ---\n";
    std::cout << "Start Symbol: " << startSymbol << "\nProductions:\n";
    for (size_t i = 0; i < productions.size(); ++i) {
        std::cout << i + 1 << ". " << productions[i].lhs << " -> ";
        if (productions[i].rhs.empty() || (productions[i].rhs.size() == 1 && productions[i].rhs[0] == EPSILON)) {
            std::cout << "epsilon\n";
        } else {
            for (size_t j = 0; j < productions[i].rhs.size(); ++j) {
                std::cout << productions[i].rhs[j] << (j < productions[i].rhs.size() - 1 ? " " : "");
            }
            std::cout << "\n";
        }
    }
}

bool Grammar::isNonTerminal(const std::string& symbol) const { return nonTerminals.find(symbol) != nonTerminals.end(); }
bool Grammar::isTerminal(const std::string& symbol) const { return terminals.find(symbol) != terminals.end(); }

void Grammar::applyLeftFactoring() {
    bool changed = true;
    while (changed) {
        changed = false;
        std::map<std::string, std::vector<std::vector<std::string>>> groupedRules;
        for (const auto& prod : productions) groupedRules[prod.lhs].push_back(prod.rhs);
        productions.clear();
        
        for (auto& pair : groupedRules) {
            std::string lhs = pair.first;
            std::vector<std::vector<std::string>>& rhsList = pair.second;
            bool factoredInThisNt = false;
            
            for (size_t i = 0; i < rhsList.size() && !factoredInThisNt; ++i) {
                for (size_t j = i + 1; j < rhsList.size() && !factoredInThisNt; ++j) {
                    std::vector<std::string> prefix;
                    size_t k = 0;
                    while (k < rhsList[i].size() && k < rhsList[j].size() && rhsList[i][k] == rhsList[j][k]) {
                        prefix.push_back(rhsList[i][k]);
                        k++;
                    }
                    if (!prefix.empty()) {
                        factoredInThisNt = true; changed = true;
                        std::vector<std::vector<std::string>> rulesWithPrefix, rulesWithoutPrefix;
                        for (const auto& rhs : rhsList) {
                            bool hasPrefix = (rhs.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), rhs.begin()));
                            if (hasPrefix) rulesWithPrefix.push_back(rhs);
                            else rulesWithoutPrefix.push_back(rhs);
                        }
                        
                        std::string newLhs = lhs + "Prime";
                        nonTerminals.insert(newLhs);
                        
                        Production baseRule; baseRule.lhs = lhs; baseRule.rhs = prefix; baseRule.rhs.push_back(newLhs);
                        productions.push_back(baseRule);
                        
                        for (const auto& rhs : rulesWithPrefix) {
                            Production leftoverRule; leftoverRule.lhs = newLhs;
                            if (rhs.size() == prefix.size()) leftoverRule.rhs.push_back(EPSILON);
                            else leftoverRule.rhs.assign(rhs.begin() + prefix.size(), rhs.end());
                            productions.push_back(leftoverRule);
                        }
                        for (const auto& rhs : rulesWithoutPrefix) {
                            Production untouchedRule; untouchedRule.lhs = lhs; untouchedRule.rhs = rhs;
                            productions.push_back(untouchedRule);
                        }
                    }
                }
            }
            if (!factoredInThisNt) {
                for (const auto& rhs : rhsList) {
                    Production originalRule; originalRule.lhs = lhs; originalRule.rhs = rhs;
                    productions.push_back(originalRule);
                }
            }
        }
    }
}

void Grammar::removeLeftRecursion() {
    std::vector<std::string> ntList(nonTerminals.begin(), nonTerminals.end());
    std::vector<Production> updatedProductions;

    for (size_t i = 0; i < ntList.size(); ++i) {
        std::string Ai = ntList[i];
        for (size_t j = 0; j < i; ++j) {
            std::string Aj = ntList[j];
            std::vector<Production> newAiProductions;

            for (auto it = productions.begin(); it != productions.end(); ++it) {
                if (it->lhs == Ai && !it->rhs.empty() && it->rhs[0] == Aj) {
                    std::vector<std::string> gamma(it->rhs.begin() + 1, it->rhs.end());
                    for (const auto& prodAj : productions) {
                        if (prodAj.lhs == Aj) {
                            Production subRule; subRule.lhs = Ai;
                            if (!(prodAj.rhs.size() == 1 && prodAj.rhs[0] == EPSILON)) subRule.rhs = prodAj.rhs;
                            subRule.rhs.insert(subRule.rhs.end(), gamma.begin(), gamma.end());
                            if (subRule.rhs.empty()) subRule.rhs.push_back(EPSILON);
                            newAiProductions.push_back(subRule);
                        }
                    }
                } else if (it->lhs == Ai) {
                    newAiProductions.push_back(*it);
                }
            }
            for (auto it = productions.begin(); it != productions.end(); ) {
                if (it->lhs == Ai) it = productions.erase(it); else ++it;
            }
            productions.insert(productions.end(), newAiProductions.begin(), newAiProductions.end());
        }

        std::vector<std::vector<std::string>> alphas, betas;
        for (const auto& prod : productions) {
            if (prod.lhs == Ai) {
                if (!prod.rhs.empty() && prod.rhs[0] == Ai) alphas.push_back(std::vector<std::string>(prod.rhs.begin() + 1, prod.rhs.end()));
                else betas.push_back(prod.rhs);
            }
        }

        if (!alphas.empty()) {
            std::string newAi = Ai + "Prime";
            nonTerminals.insert(newAi);
            for (const auto& beta : betas) {
                Production p; p.lhs = Ai;
                if (beta.size() == 1 && beta[0] == EPSILON) p.rhs.push_back(newAi);
                else { p.rhs = beta; p.rhs.push_back(newAi); }
                updatedProductions.push_back(p);
            }
            for (const auto& alpha : alphas) {
                Production p; p.lhs = newAi; p.rhs = alpha; p.rhs.push_back(newAi);
                updatedProductions.push_back(p);
            }
            Production pEps; pEps.lhs = newAi; pEps.rhs.push_back(EPSILON);
            updatedProductions.push_back(pEps);
        } else {
            for (const auto& prod : productions) if (prod.lhs == Ai) updatedProductions.push_back(prod);
        }
    }
    productions = updatedProductions;
}
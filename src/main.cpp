#include "grammar.h"
#include "first_follow.h"
#include "parser.h"
#include <iostream>

void processGrammarAndInput(const std::string& grammarFile, const std::string& inputFile) {
    Grammar myGrammar;
    if (!myGrammar.loadFromFile(grammarFile)) {
        std::cerr << "Failed to load " << grammarFile << "\n";
        return;
    }
    
    myGrammar.removeLeftRecursion();
    myGrammar.applyLeftFactoring();
    myGrammar.printGrammar();

    FirstFollow ff;
    ff.computeFirstSets(myGrammar);
    ff.computeFollowSets(myGrammar);
    ff.printFirstSets();
    ff.printFollowSets();

    Parser parser;
    parser.buildParsingTable(myGrammar, ff);
    parser.printParsingTable(myGrammar);

    std::ifstream file(inputFile);
    std::string line;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            TreeNode* root = parser.parseString(line, myGrammar, ff);
            if (root != nullptr) {
                std::cout << "\n=== AST ===\n";
                printTree(root);
                destroyTree(root); // Prevent Memory Leaks!
            }
        }
    } else {
        std::cout << "\n(No input file provided or found, skipping string parsing)\n";
    }
}

int main() {
    std::cout << "--- CS4031 LL(1) Parser Engine ---\n";
    
    // Test the main Expression Grammar with valid strings
    processGrammarAndInput("input/grammar2.txt", "input/input_valid.txt");
    
    // Process Invalid Strings to test Error Recovery
    std::cout << "\n\n*** TESTING ERROR RECOVERY ***\n";
    processGrammarAndInput("input/grammar2.txt", "input/input_errors.txt");

    return 0;
}
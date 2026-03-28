#include "grammar.h"
#include "first_follow.h" // Add this!
#include "parser.h" // Add this!


int main() {
    Grammar myGrammar;
    
    if (myGrammar.loadFromFile("grammar.txt")) {
        // ... (your existing transformations) ...
        myGrammar.removeLeftRecursion();
        myGrammar.applyLeftFactoring();
        
        std::cout << "\n--- Transformed Grammar ---\n";
        myGrammar.printGrammar();
        
        // --- NEW CODE ---
        FirstFollow ff;
        ff.computeFirstSets(myGrammar);
        ff.printFirstSets();
        
        ff.computeFollowSets(myGrammar);
        ff.printFollowSets();

        // --- NEW CODE in main.cpp ---
        Parser parser;
        parser.buildParsingTable(myGrammar, ff);
        parser.printParsingTable(myGrammar);
        // --- NEW CODE in main.cpp ---
        // Test Valid String
        std::string validTest = "id + id $";
        parser.parseString(validTest, myGrammar, ff);

        // Test Invalid String (From Assignment 5.4)
        std::string invalidTest = "id + * id $";
        parser.parseString(invalidTest, myGrammar, ff);
    }
    
    return 0;
}
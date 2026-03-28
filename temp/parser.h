#ifndef PARSER_H
#define PARSER_H

#include "grammar.h"
#include "first_follow.h"
#include <map>
#include <string>

class Parser {
public:
    // The 2D Parsing Table: Map<Row, Map<Column, Production>>
    std::map<std::string, std::map<std::string, Production>> parsingTable;
    
    // Flag to check if the grammar is actually LL(1)
    bool isLL1Grammar = true;

    // Build the table using the grammar and the sets
    void buildParsingTable(const Grammar& grammar, const FirstFollow& ff);
    
    // Print the table in a grid format
    void printParsingTable(const Grammar& grammar) const;

    // Add this under your existing public methods:
    void parseString(const std::string& inputLine, const Grammar& grammar, const FirstFollow& ff);

private:
    // Helper to calculate FIRST(alpha) where alpha is a whole right-hand side
    std::set<std::string> getFirstOfSequence(const std::vector<std::string>& rhs, const FirstFollow& ff, const Grammar& grammar);
};

#endif // PARSER_H
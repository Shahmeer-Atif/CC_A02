#ifndef PARSER_H
#define PARSER_H

#include "grammar.h"
#include "first_follow.h"
#include "stack.h"
#include <map>
#include <string>

class Parser {
public:
    std::map<std::string, std::map<std::string, Production>> parsingTable;
    bool isLL1Grammar = true;

    void buildParsingTable(const Grammar& grammar, const FirstFollow& ff);
    void printParsingTable(const Grammar& grammar) const;
    TreeNode* parseString(const std::string& inputLine, const Grammar& grammar, const FirstFollow& ff);

private:
    std::set<std::string> getFirstOfSequence(const std::vector<std::string>& rhs, const FirstFollow& ff, const Grammar& grammar);
};

#endif
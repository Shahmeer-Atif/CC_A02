#include "error_handler.h"

void ErrorHandler::handlePanicMode(const std::string& unexpectedToken, const std::string& nonTerminal, 
                                   const FirstFollow& ff, Stack& stack, size_t& inputPtr) {
    bool inFollow = false;
    auto followIt = ff.followSets.find(nonTerminal);
    if (followIt != ff.followSets.end() && followIt->second.find(unexpectedToken) != followIt->second.end()) {
        inFollow = true;
    }

    if (inFollow) {
        std::cout << std::string(61, ' ') << "PANIC: Popping '" << nonTerminal << "' to sync.\n";
        stack.pop();
    } else {
        std::cout << std::string(61, ' ') << "PANIC: Skipping rogue '" << unexpectedToken << "'\n";
        inputPtr++;
    }
}
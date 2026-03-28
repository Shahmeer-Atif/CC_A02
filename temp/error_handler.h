#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <set>
#include "stack.h"

class ErrorHandler {
public:
    // Handles missing/unexpected symbols using Panic Mode
    static void handlePanicMode(const std::string& unexpectedToken, 
                                const std::string& currentNonTerminal, 
                                const std::set<std::string>& followSet, 
                                Stack& stack, 
                                size_t& inputPtr);
                                
    // Handles terminal mismatches
    static void handleTerminalMismatch(const std::string& expected, 
                                       const std::string& found, 
                                       Stack& stack);
};

#endif
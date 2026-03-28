#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <set>
#include <iostream>
#include "stack.h"
#include "first_follow.h"

class ErrorHandler {
public:
    static void handlePanicMode(const std::string& unexpectedToken, const std::string& nonTerminal, 
                                const FirstFollow& ff, Stack& stack, size_t& inputPtr);
};

#endif
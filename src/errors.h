#ifndef LK_ERRORS
#define LK_ERRORS

#include <iostream>
#include <string>


enum class ErrorKind {
    InstructionDoesNotAcceptBlock
};

struct CompilationError {
    ErrorKind kind;
    std::string message;

    bool operator==(const CompilationError& err) const;
    friend std::ostream& operator<<(std::ostream& os, const CompilationError& line);
};

CompilationError instruction_does_not_accept_block();

#endif
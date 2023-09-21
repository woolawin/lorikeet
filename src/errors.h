#ifndef LK_ERRORS
#define LK_ERRORS

#include <iostream>
#include <string>


struct InstructionDoesNotAcceptBlock {
    std::string message() const;
    bool operator==(const InstructionDoesNotAcceptBlock& err) const;
};

enum class ErrorKind {
    InstructionDoesNotAcceptBlock
};

struct CompilationError {

    ErrorKind kind;
    union {
        InstructionDoesNotAcceptBlock instruction_does_not_accept_block;
    } data;

    bool operator==(const CompilationError& err) const;
};

CompilationError instruction_does_not_accept_block();




#endif
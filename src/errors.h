#ifndef LK_ERRORS
#define LK_ERRORS

#include <iostream>
#include <string>


enum class ErrorKind {
    InstructionDoesNotAcceptBlock,
    InvalidIndentation,
    UnknownInstruction
};

struct CompilationError {
    ErrorKind kind;
    size_t line_num;
    std::string message;

    bool operator==(const CompilationError& err) const;
    friend std::ostream& operator<<(std::ostream& os, const CompilationError& line);
};

CompilationError instruction_does_not_accept_block(size_t line_num);
CompilationError invalid_indentation(size_t line_num);
CompilationError unknown_instruction(size_t line_num);

#endif
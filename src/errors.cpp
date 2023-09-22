#include "errors.h"

std::ostream& operator<<(std::ostream& os, const CompilationError& err) {
    os << "{message=\"" << err.message << "\"}";
    return os;
}


CompilationError instruction_does_not_accept_block() {
    CompilationError compile_err = {};
    compile_err.kind = ErrorKind::InstructionDoesNotAcceptBlock;
    compile_err.message = "The current instruction does open a block, place this line on the same level";
    return compile_err;
}

bool CompilationError::operator==(const CompilationError& other) const {
    return this->kind == other.kind
        && this->message == other.message;
}
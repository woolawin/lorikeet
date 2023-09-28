#include "errors.h"

std::ostream& operator<<(std::ostream& os, const CompilationError& err) {
    os << "{line=" << err.line_num << " ,message=\"" << err.message << "\"}";
    return os;
}


CompilationError instruction_does_not_accept_block(size_t line_num) {
    CompilationError compile_err = {};
    compile_err.line_num = line_num;
    compile_err.kind = ErrorKind::InstructionDoesNotAcceptBlock;
    compile_err.message = "The current instruction does open a block, place this line on the same level";
    return compile_err;
}

CompilationError invalid_indentation(size_t line_num) {
    CompilationError compile_err = {};
    compile_err.line_num = line_num;
    compile_err.kind = ErrorKind::InvalidIndentation;
    compile_err.message = "The indentation for this line does not correspond to a previous line";
    return compile_err;
}

CompilationError unknown_instruction(size_t line_num) {
    CompilationError compile_err = {};
    compile_err.line_num = line_num;
    compile_err.kind = ErrorKind::UnknownInstruction;
    compile_err.message = "Instruction could not be find with name";
    return compile_err;
}

bool CompilationError::operator==(const CompilationError& other) const {
    return this->line_num == other.line_num
        && this->kind == other.kind
        && this->message == other.message;
}
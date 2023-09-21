#include "errors.h"

//std::ostream& operator<<(std::ostream& os, const CompilationError& err) {
//    err.print(os);
//    return os;
//}

//void InstructionDoesNotAcceptBlock::print(std::ostream& os) const {
//}

CompilationError compile_error(InstructionDoesNotAcceptBlock err) {
    CompilationError compile_err = {};
    compile_err.kind = ErrorKind::InstructionDoesNotAcceptBlock;
    compile_err.data.instruction_does_not_accept_block = err;
    return compile_err;
}

bool InstructionDoesNotAcceptBlock::operator==(const InstructionDoesNotAcceptBlock& other) const {
    return true;
}

std::string InstructionDoesNotAcceptBlock::message() const {
    return "The current instruction does open a block, place this line on the same level";
}

#define case_eq(x) (this->data.x == other.data.x);

bool CompilationError::operator==(const CompilationError& other) const {
    if (this->kind != other.kind) {
        return false;
    }
    switch (this->kind) {
    case ErrorKind::InstructionDoesNotAcceptBlock:
        return case_eq(instruction_does_not_accept_block);
    }

    return false;
}
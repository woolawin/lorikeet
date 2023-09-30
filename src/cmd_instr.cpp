#include <iostream>

#include "cmd_instr.h"

bool CommandInstr::operator==(const CommandInstr& other) const {
    return this->name == other.name
        && this->path == other.path
        && this->id == other.id;
}

std::ostream& operator<<(std::ostream& os, const CommandInstr& instr) {
    os << "{ id=" << instr.id << ", name=\"" << instr.name << "\", path=\"" << instr.path << "\" }" << std::endl;
    return os;
}
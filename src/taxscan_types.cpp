#include "taxscan.h"

bool TaxScanError::operator==(const TaxScanError& other) const {
    return this->message == other.message
        && this->is_eof_error == other.is_eof_error;
}

std::ostream& operator<<(std::ostream& os, const TaxScanError& err) {
    os << "TaxScanError(message=\"" << err.message << "\" is_eof=" << err.is_eof_error << ")";
    return os;
}

bool RoutineTaxonomy::operator==(const RoutineTaxonomy& other) const {
    return this->instructions == other.instructions;
}

std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& routine) {
    os << "RoutineTaxonomy(\n\tinstructions=[";
    for (size_t i = 0; i < routine.instructions.size(); i++) {
        os << "\n\n" << routine.instructions[i];
    }
    os << "\n\t]\n)";
    return os;
}

bool BranchTaxonomy::operator==(const BranchTaxonomy& other) const {
    return this->default_branch == other.default_branch
        && this->input == other.input
        && this->routine == other.routine;
}

std::ostream& operator<<(std::ostream& os, const BranchTaxonomy& branch) {
    os << "{\n\tdefault_branch=" << branch.default_branch << "\n\tinput=" << branch.input;
    os << "\n\troutine=" << branch.routine << "\n}";
    return os;
}

bool InstructionTaxonomy::operator==(const InstructionTaxonomy& other) const {
    return this->name == other.name
        && this->input == other.input
        && this->branches == other.branches;
}

std::ostream& operator<<(std::ostream& os, const InstructionTaxonomy& instr) {
    os << "{\n\tname=\"" << instr.name << "\"\n\tinput=[";
    for (size_t i = 0; i < instr.input.size(); i++) {
        os << "\n\t\t" << instr.input[i].raw();
    }
    os << "\n\t]\n\tbranches=[";
    for (size_t i = 0; i < instr.branches.size(); i++) {
        os << "\n\n" << instr.branches[i];
    }
    os << "\n\t]\n)";
    return os;
}

bool FileTaxonomy::operator==(const FileTaxonomy& other) const {
    return this->routine == other.routine
        && this->err == other.err;
}

std::ostream& operator<<(std::ostream& os, const FileTaxonomy& file) {
    os << "FileTaxonomy(\n\troutine=" << file.routine << "\n\terr=" << "" << "\n)";
    return os;
}

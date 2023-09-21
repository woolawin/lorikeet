#include <sstream>
#include "taxscan.h"

std::string repeat(const std::string value, int count) {
    std::stringstream stream;
    for (int i = 0; i < count; i++) {
        stream << value;
    }
    return stream.str();
}

std::string indent(int count) {
    return repeat(" ", count * 2);
}

void to_stream(std::ostream& os, const RoutineTaxonomy& routine, int indentation);
void to_stream(std::ostream& os, const InstructionTaxonomy& instr, int indentation);
void to_stream(std::ostream& os, const BranchTaxonomy& branch, int indentation);

std::string to_string(bool boolean) {
    return boolean ? "true" : "false";
}
/*
bool TaxScanError::operator==(const TaxScanError& other) const {
    return this->message == other.message
        && this->is_eof_error == other.is_eof_error;
}

std::ostream& operator<<(std::ostream& os, const TaxScanError& err) {
    os << "{message=\"" << err.message << "\" is_eof=" << to_string(err.is_eof_error) << "}";
    return os;
}
*/
bool RoutineTaxonomy::operator==(const RoutineTaxonomy& other) const {
    return this->instructions == other.instructions;
}

std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& routine) {
    to_stream(os, routine, 0);
    return os;
}

void to_stream(std::ostream& os, const RoutineTaxonomy& routine, int indentation) {
    os << "{" << std::endl;
    os << indent(indentation + 1) << "instructions=[" << std::endl;
    for (size_t idx = 0; idx < routine.instructions.size(); idx++) {
        to_stream(os, routine.instructions[idx], indentation + 2);
    }
    os << indent(indentation + 1) << "]" << std::endl;
    os << indent(indentation) << "}" << std::endl;
}

bool BranchTaxonomy::operator==(const BranchTaxonomy& other) const {
    return this->default_branch == other.default_branch
        && this->input == other.input
        && this->routine == other.routine;
}

std::ostream& operator<<(std::ostream& os, const BranchTaxonomy& branch) {
    to_stream(os, branch, 0);
    return os;
}

void to_stream(std::ostream& os, const BranchTaxonomy& branch, int indentation) {
    os << indent(indentation) << "{" << std::endl;
    os << indent(indentation + 1) << "default_branch=" << to_string(branch.default_branch) << std::endl;
    os << indent(indentation + 1) << "input=\"" << branch.input.raw() << "\"" << std::endl;
    os << indent(indentation + 1) << "routine=";
    to_stream(os, branch.routine, indentation + 2);
    os << indent(indentation) << "}" << std::endl;
}

bool InstructionTaxonomy::operator==(const InstructionTaxonomy& other) const {
    return this->name == other.name
        && this->input == other.input
        && this->branches == other.branches;
}

std::ostream& operator<<(std::ostream& os, const InstructionTaxonomy& instr) {
    to_stream(os, instr, 0);
    return os;
}

void to_stream(std::ostream& os, const InstructionTaxonomy& instr, int indentation) {
    os << indent(indentation) << "{" << std::endl;
    os << indent(indentation + 1) << "name=\"" << instr.name << "\"" << std::endl;
    os << indent(indentation + 1) << "input=[" << std::endl;
    for (size_t idx = 0; idx < instr.input.size(); idx++) {
         os << indent(indentation + 2) << "\"" << instr.input[idx].raw() << "\"" << std::endl;
    }
    os << indent(indentation + 1) << "]" << std::endl;
    os << indent(indentation + 1) << "branches=[" << std::endl;
    for (size_t idx = 0; idx < instr.branches.size(); idx++) {
        to_stream(os, instr.branches[idx], indentation + 2);
    }
    os << indent(indentation + 1) << "]" << std::endl;
    os << indent(indentation) << "}" << std::endl;
}

bool FileTaxonomy::operator==(const FileTaxonomy& other) const {
    return this->routine == other.routine
        && this->errors == other.errors;
}

std::ostream& operator<<(std::ostream& os, const FileTaxonomy& file) {
    os << std::endl << "{" << std::endl;
    os << indent(1) << "routine=";
    to_stream(os, file.routine, 1);
    if (!file.errors.empty()) {
        // TODO print errors
        //    os << indent(1) << "err=";
        //os << file.err << std::endl;
    }
    os << "}" << std::endl;
    return os;
}

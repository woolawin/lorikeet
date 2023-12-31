#include<sstream>

#include "core_types.h"

TaxStrat value_strat() {
    return { .parse_strat = ParseStrat::Value, .block_function = BlockFunction::NA };
}

TaxStrat command_strat() {
    return { .parse_strat = ParseStrat::Command, .block_function = BlockFunction::Append };
}

TaxStrat branch_strat(std::vector<std::string> branch_instr) {
    return { .parse_strat = ParseStrat::Branch, .block_function = BlockFunction::Routine, .branch_instr = branch_instr };
}

TaxStrat custom_strat(BlockFunction block_func) {
    return { .parse_strat = ParseStrat::Custom, .block_function = block_func };
}

InstructionID RandomIDGenerator::new_instr_id() {
    return this->distribution(this->rng);
}

BranchTaxonomy new_branch(bool is_default, const Line& input) {
    return {
	    .default_branch = is_default,
	    .input = input,
		.routine = { .statements = {} }
	};
}

BranchTaxonomy& StatementTaxonomy::branch(bool is_default, const Line& line) {
    this->branches.push_back(new_branch(is_default, line));
    return this->branches.back();
}

StatementTaxonomy new_statement(InstructionID instr_id, const Line& line) {
	return {
		.name = line.first_word(),
		.input =  {line.crop_from_first_word()},
		.instr_id = instr_id,
		.branches = {},
	};
}

StatementTaxonomy& RoutineTaxonomy::append(InstructionID instr_id, const Line& line) {
    this->statements.push_back(new_statement(instr_id, line));
    return this->statements.back();
}

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

void to_stream(std::ostream& os, const StatementTaxonomy& instr, int indentation);
void to_stream(std::ostream& os, const BranchTaxonomy& branch, int indentation);

std::string to_string(bool boolean) {
    return boolean ? "true" : "false";
}

bool RoutineTaxonomy::operator==(const RoutineTaxonomy& other) const {
    return this->statements == other.statements;
}

std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& routine) {
    to_stream(os, routine, 0);
    return os;
}

void to_stream(std::ostream& os, const RoutineTaxonomy& routine, int indentation) {
    os << "{" << std::endl;
    os << indent(indentation + 1) << "statements=[" << std::endl;
    for (size_t idx = 0; idx < routine.statements.size(); idx++) {
        to_stream(os, routine.statements[idx], indentation + 2);
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

bool StatementTaxonomy::operator==(const StatementTaxonomy& other) const {
    return this->name == other.name
        && this->input == other.input
        && this->instr_id == other.instr_id
        && this->branches == other.branches;
}

std::ostream& operator<<(std::ostream& os, const StatementTaxonomy& instr) {
    to_stream(os, instr, 0);
    return os;
}

void to_stream(std::ostream& os, const StatementTaxonomy& stmt, int indentation) {
    os << indent(indentation) << "{" << std::endl;
    os << indent(indentation + 1) << "name=\"" << stmt.name << "\"" << std::endl;
    os << indent(indentation + 1) << "input=[" << std::endl;
    for (size_t idx = 0; idx < stmt.input.size(); idx++) {
         os << indent(indentation + 2) << "\"" << stmt.input[idx].raw() << "\"" << std::endl;
    }
    os << indent(indentation + 1) << "]" << std::endl;
    os << indent(indentation + 1) << "instr_id=" << stmt.instr_id << std::endl;
    os << indent(indentation + 1) << "branches=[" << std::endl;
    for (size_t idx = 0; idx < stmt.branches.size(); idx++) {
        to_stream(os, stmt.branches[idx], indentation + 2);
    }
    os << indent(indentation + 1) << "]" << std::endl;
    os << indent(indentation) << "}" << std::endl;
}


#include <optional>
#include <algorithm>
#include "taxscan.h"

FileTaxonomy empty_file_taxonomy() {
	return { .routine = { .instructions = {} } };
}

FileTaxonomy err_file(std::optional<TaxScanError>& err) {
    return { .routine = { .instructions = {} }, .err = err };
}

BranchTaxonomy new_branch(bool is_default, const Line& input) {
    return {
	    .default_branch = is_default,
	    .input = input,
		.routine = { .instructions = {} }
	};
}

InstructionTaxonomy new_instruction(const Line& line) {
	return {
		.name = line.first_word(),
		.input =  {line.crop_from_first_word()},
		.branches = {},
	};
}

BranchTaxonomy& InstructionTaxonomy::branch(bool is_default, const Line& line) {
    this->branches.push_back(new_branch(is_default, line));
    return this->branches.back();
}

InstructionTaxonomy& RoutineTaxonomy::append(const Line& line) {
    this->instructions.push_back(new_instruction(line));
    return this->instructions.back();
}

void append_input(InstructionTaxonomy& instr, const std::vector<Line>& lines, int start, int count) {
    for (int idx = start; idx < start + count; idx++) {
        instr.input.push_back(lines[idx]);
	}
}


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

struct BlockResult {
    std::vector<Line> lines;
    size_t resume_at;
};

void trim_lines(std::vector<Line>& lines);

bool skip_line(const Line& line, bool& is_multi_line_comment);
BlockResult scan_block(const std::vector<Line>& lines, size_t starting_from, const Indentation& indentation);
void scan_routine(const std::vector<Line>& lines, Indentation& indentation, RoutineTaxonomy& routine, Agent& agent);
size_t scan_branches(const std::vector<Line>& lines, size_t from, TaxStrat tax_strat, Indentation& indentation, InstructionTaxonomy& instr, Agent& agent);

FileTaxonomy scan_file(const std::vector<std:: string>& lines_raw, Agent& agent) {
    FileTaxonomy file = empty_file_taxonomy();

    Indentation indentation;
    std::vector<Line> lines;
    parse(lines_raw, lines);
    scan_routine(lines, indentation, file.routine, agent);
    return file;
}

void scan_routine(const std::vector<Line>& lines, Indentation& indentation, RoutineTaxonomy& routine, Agent& agent) {
    bool is_multi_line_comment = false;
    for (size_t idx = 0; idx < lines.size(); idx++) {
        const Line line = lines[idx];
        if (skip_line(line, is_multi_line_comment)) {
            continue;
        }
        InstructionTaxonomy& instr = routine.append(line);
        if (idx == lines.size() - 1) {
            continue;
        }
        const std::string starting_whitespace = lines[idx + 1].starting_whitespace();
        int indentation_diff = indentation.diff(starting_whitespace);
        if (indentation_diff == SAME) {
            continue;
        }
        if (indentation_diff == ERROR) {
            continue; // handle error
        }

        TaxStrat tax_strat = agent.tax_strat(line.first_word());
        if (tax_strat.block_function == BlockFunction::NA) {
            continue; //error
        }
        BlockResult result = scan_block(lines, idx + 1, indentation);
        idx = result.resume_at;

        if (tax_strat.block_function == BlockFunction::Append) {
            trim_lines(result.lines);
            Line first_input = line.crop_from_first_word();
            if (!first_input.empty()) {
                result.lines.insert(result.lines.begin(), first_input);
            }
            instr.input = result.lines;
            continue;
        }

        // We must be in routine
        BranchTaxonomy& branch = instr.branch(true, parse(0, ""));
        Indentation new_indentation = indentation.indent(starting_whitespace);
        scan_routine(result.lines, new_indentation, branch.routine, agent);
        idx = scan_branches(lines, idx, tax_strat, indentation, instr, agent);
    }
}

size_t scan_branches(const std::vector<Line>& lines, size_t from, TaxStrat tax_strat, Indentation& indentation, InstructionTaxonomy& instr, Agent& agent) {
    bool is_multi_line_comment = false;
    for (size_t idx = from; idx < lines.size() - 1; idx++) {
        Line next_line = lines[idx + 1];
        if (skip_line(next_line, is_multi_line_comment)) {
            continue;
        }
        const bool next_instr_is_branch = std::find(
            tax_strat.branch_instr.begin(),
            tax_strat.branch_instr.end(),
            next_line.first_word()
         ) != tax_strat.branch_instr.end();

        if (!next_instr_is_branch) {
            return idx;
        }

        BlockResult result = scan_block(lines, idx + 2, indentation);
        idx = result.resume_at;
        if (result.lines.empty()) {
            continue;
        }

        BranchTaxonomy& branch = instr.branch(false, next_line.crop_from_first_word());
        Indentation new_indentation = indentation.indent(result.lines[0].starting_whitespace());
        scan_routine(result.lines, new_indentation, branch.routine, agent);
    }
    return lines.size();
}

BlockResult scan_block(const std::vector<Line>& lines, size_t starting_from, const Indentation& indentation) {
    std::vector<Line> block;
    bool is_multi_line_comment = false;
    for (size_t idx = starting_from; idx < lines.size(); idx++) {
        const Line line = lines[idx];
        if (skip_line(line, is_multi_line_comment)) {
            continue;
        }
        const int diff = indentation.diff(line.starting_whitespace());
        if (diff == ERROR) {
            return { .lines = block, .resume_at = idx - 1 }; // @TODO handle error
        }
        if (diff == INCREASE) {
            block.push_back(line);
            continue;
        }
        if (diff == DECREASE || line.first_word() != "end") {
            return { .lines = block, .resume_at = idx - 1 };
        }
        return { .lines = block, .resume_at = idx };
    }
    return { .lines = block, .resume_at = lines.size() };
}

bool skip_line(const Line& line, bool& is_multi_line_comment) {
    if (line.only_whitespace()) {
        return true;
    }
    if (is_multi_line_comment && line.ends_with_symbol_seq(">#")) {
        is_multi_line_comment = false;
        return true;
    }
    if (is_multi_line_comment) {
        return true;
    }

    if (line.starts_with_symbol_seq("#<")) {
        is_multi_line_comment = true;
        return true;
    }

    if (line.starts_with_symbol_seq("#")) {
        return true;
    }
    return false;
}

void trim_lines(std::vector<Line>& lines) {
    for (int idx = 0; idx < lines.size(); idx++) {
        lines[idx] = lines[idx].trim();
    }
}

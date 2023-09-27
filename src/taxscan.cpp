
#include "taxscan.h"

FileTaxonomy empty_file_taxonomy() {
	return { .routine = { .instructions = {} } };
}

FileTaxonomy err_file(std::vector<CompilationError> errors) {
    return { .routine = { .instructions = {} }, .errors = errors };
}

void append_input(InstructionTaxonomy& instr, const std::vector<Line>& lines, int start, int count) {
    for (int idx = start; idx < start + count; idx++) {
        instr.input.push_back(lines[idx]);
	}
}

struct BlockResult {
    std::vector<Line> lines;
    size_t resume_at;
    std::vector<CompilationError> errors;
};

struct BranchResult {
    size_t resume_at;
    std::vector<CompilationError> errors;
};

void trim_lines(std::vector<Line>& lines);

bool skip_line(const Line& line, bool& is_multi_line_comment);
BlockResult scan_block(const std::vector<Line>& lines, size_t starting_from, const Indentation& indentation);
std::vector<CompilationError> scan_routine(const std::vector<Line>& lines, Indentation& indentation, RoutineTaxonomy& routine, StateMachine& machine);
BranchResult scan_branches(const std::vector<Line>& lines, size_t from, TaxStrat tax_strat, Indentation& indentation, InstructionTaxonomy& instr, StateMachine& machine);

FileTaxonomy scan_file(const std::vector<std:: string>& lines_raw, StateMachine& machine) {
    FileTaxonomy file = empty_file_taxonomy();

    Indentation indentation;
    std::vector<Line> lines;
    parse(lines_raw, lines);
    std::vector<CompilationError> errors = scan_routine(lines, indentation, file.routine, machine);
    if (!errors.empty()) {
        return err_file(errors);
    }
    return file;
}

std::vector<CompilationError> scan_routine(const std::vector<Line>& lines, Indentation& indentation, RoutineTaxonomy& routine, StateMachine& machine) {
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
        const IndentationDiff indentation_diff = indentation.diff(starting_whitespace);
        if (indentation_diff == IndentationDiff::Same) {
            continue;
        }
        if (indentation_diff == IndentationDiff::Error) {
            std::cout << line.raw() << std::endl;
            return { invalid_indentation(idx + 1) };
        }

        TaxStrat tax_strat = machine.tax_strat(line.first_word());
        if (tax_strat.block_function == BlockFunction::NA) {
            return { instruction_does_not_accept_block(idx + 2) };
        }
        BlockResult result = scan_block(lines, idx + 1, indentation);
        if (!result.errors.empty()) {
            return result.errors;
        }
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
        std::vector<CompilationError> errors = scan_routine(result.lines, new_indentation, branch.routine, machine);
        if (!errors.empty()) {
            return errors;
        }
        BranchResult branch_result = scan_branches(lines, idx, tax_strat, indentation, instr, machine);
        if (!branch_result.errors.empty()) {
            return branch_result.errors;
        }
        idx = branch_result.resume_at;
    }
    return {};
}

BranchResult scan_branches(const std::vector<Line>& lines, size_t from, TaxStrat tax_strat, Indentation& indentation, InstructionTaxonomy& instr, StateMachine& machine) {
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
            return { .resume_at = idx, .errors = {} };
        }

        BlockResult result = scan_block(lines, idx + 2, indentation);
        if (!result.errors.empty()) {
            return { .resume_at = lines.size(), .errors = result.errors };
        }
        idx = result.resume_at;
        if (result.lines.empty()) {
            continue;
        }

        BranchTaxonomy& branch = instr.branch(false, next_line.crop_from_first_word());
        Indentation new_indentation = indentation.indent(result.lines[0].starting_whitespace());
        std::vector<CompilationError> errors = scan_routine(result.lines, new_indentation, branch.routine, machine);
        if (!errors.empty()) {
            return { .resume_at = lines.size(), .errors = errors };
        }
    }
    return { .resume_at = lines.size(), .errors = {} };
}

BlockResult scan_block(const std::vector<Line>& lines, size_t starting_from, const Indentation& indentation) {
    std::vector<Line> block;
    bool is_multi_line_comment = false;
    for (size_t idx = starting_from; idx < lines.size(); idx++) {
        const Line line = lines[idx];
        if (skip_line(line, is_multi_line_comment)) {
            continue;
        }
        const IndentationDiff diff = indentation.diff(line.starting_whitespace());
        if (diff == IndentationDiff::Error) {
            return { .lines = block, .resume_at = idx - 1, .errors = { invalid_indentation(idx + 1) } };
        }
        if (diff == IndentationDiff::Increase) {
            block.push_back(line);
            continue;
        }
        if (diff == IndentationDiff::Decrease || line.first_word() != "end") {
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

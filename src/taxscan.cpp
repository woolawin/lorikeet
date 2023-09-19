
#include <optional>
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

struct BlockResult {
    std::vector<Line> lines;
    size_t resume_at;
};

bool skip_line(const Line& line, bool& is_multi_line_comment);
BlockResult scan_block(const std::vector<Line>& lines, size_t starting_from, const Indentation& indentation);
void scan_routine(const std::vector<Line>& lines, Indentation& indentation, RoutineTaxonomy& routine, Agent& agent);

FileTaxonomy scan_file(const std::vector<std:: string>& lines_raw, Agent& agent) {
    FileTaxonomy file = empty_file_taxonomy();

    Indentation indentation;
    std::vector<Line> lines;
    for (size_t idx = 0; idx < lines_raw.size(); idx++) {
        lines.push_back(parse(lines_raw[idx]));
    }
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
        if (indentation_diff == 0) {
            continue;
        }
        if (indentation_diff == 2) {
            continue; // handle error
        }
        if (indentation_diff < 0) {
            continue; // goback
        }

        TaxStrat tax_strat = agent.tax_strat(line.first_word());
        if (tax_strat.block_kind == NA) {
            continue; //error
        }
        BlockResult result = scan_block(lines, idx + 1, indentation);
        idx = result.resume_at;

        if (tax_strat.block_kind == INPUT) {
            instr.input = result.lines;
        }

        if (tax_strat.block_kind == APPEND) {
            Line input = line.crop_from_first_word();
            input.append(' ');
            for (size_t i = 0; i < result.lines.size(); i++) {
                input.append(result.lines[i].trim());
                if (i != result.lines.size() - 1) {
                    input.append(' ');
                }
            }
            instr.input = {input};
        }

        if (tax_strat.block_kind == ROUTINE) {
            BranchTaxonomy& branch = instr.branch(true, parse(""));
            Indentation new_indentation = indentation.indent(starting_whitespace);
            scan_routine(result.lines, new_indentation, branch.routine, agent);
        }
    }
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
        if (diff == 2) {
            return { .lines = block, .resume_at = idx - 1 }; // @TODO handle error
        }
        if (diff == 1) {
            block.push_back(line);
            continue;
        }
        if (diff < 0 || line.first_word() != "end") {
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

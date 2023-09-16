
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
		.input =  {line},
		.branches = {},
	};
}

void InstructionTaxonomy::branch(bool is_default, const Line& line) {
    this->branches.push_back(new_branch(is_default, line));
}

BranchTaxonomy& InstructionTaxonomy::current_branch() {
	return this->branches.back();
}

InstructionTaxonomy& RoutineTaxonomy::append(const Line& line) {
    this->instructions.push_back(new_instruction(line));
    return this->instructions.back();
}

InstructionTaxonomy& RoutineTaxonomy::current_instr() {
    return this->instructions.back();
}

void append_input(InstructionTaxonomy& instr, const std::vector<std::string>& lines, int start, int count) {
	for (int idx = start; idx < start + count; idx++) {
		instr.input.push_back(parse(lines[idx]));
	}
}

struct ScanRoutineResult {
    size_t offset;
    std::optional<TaxScanError> err;
};

ScanRoutineResult scan_routine(const std::vector<std::string>& lines, size_t position, Agent& agent, InstructionTaxonomy& instr);
bool skip_line(const Line& line, bool& is_multi_line_comment);

struct DefaultPeeker: public Peek {
    const std::vector<std::string>& lines;
    size_t offset;

    public:
    DefaultPeeker(const std::vector<std::string>& lines, size_t offset) : lines(lines), offset(offset) {}

    const std::optional<Line> peek() override {
        if (this->offset == this->lines.size() - 1) {
            return std::nullopt;
        }
        this->offset++;
        return parse(this->lines[this->offset]);
    }
};

FileTaxonomy scan_file(const std::vector<std:: string>& lines, Agent& agent) {
    FileTaxonomy file = empty_file_taxonomy();

    bool is_multi_line_comment = false;
    for (size_t idx = 0; idx < lines.size(); idx++) {
        const Line line = parse(lines[idx]);
        if (skip_line(line, is_multi_line_comment)) {
            continue;
        }

        InstructionTaxonomy& instr = file.routine.append(line);
        DefaultPeeker peek = DefaultPeeker(lines, idx);
        StepResult result = agent.step(instr, peek);
        if (result.err != std::nullopt) {
            return err_file(result.err);
        }

        if (result.input_count > 1) {
            append_input(instr, lines, idx + 1, result.input_count);
        }

        idx += result.offset;

        if (result.branch != std::nullopt) {
            file.routine.current_instr().branch(result.branch->is_default, line);
            ScanRoutineResult routine_result = scan_routine(lines, idx + 1, agent, instr);
            if (routine_result.err != std::nullopt) {
                return err_file(routine_result.err);
            }

            idx = routine_result.offset;
        }
    }
    return file;
}

ScanRoutineResult scan_routine(const std::vector<std::string>& lines, size_t position, Agent& agent, InstructionTaxonomy& instr) {
    bool is_multi_line_comment = false;
    const size_t end = lines.size();
    if (position == end) {
        return { .err = (TaxScanError){ .message = "", .is_eof_error = true} };
    }
    for (size_t idx = position; idx < end; idx++) {
        const Line line = parse(lines[idx]);
        if (skip_line(line, is_multi_line_comment)) {
            continue;
        }

        SubroutineResult result = agent.subroutine(instr, line);

        if (result.err != std::nullopt) {
            return { .err = result.err };
        }

        if (result.add) {
            instr.current_branch().routine.append(line);
        }

        if (result.branch != std::nullopt) {
            instr.branch(result.branch->is_default, line);
            idx += result.offset;
            continue;
        }

        if (result.step) {
			InstructionTaxonomy& sub_instr = instr.current_branch().routine.current_instr();
			DefaultPeeker peek = DefaultPeeker(lines, idx);
			StepResult step_result = agent.step(sub_instr, peek);

		    if (step_result.err != std::nullopt) {
				return { .err = step_result.err };
			}
			if (step_result.input_count > 1) {
				append_input(sub_instr, lines, idx + 1, step_result.input_count);
			}

		    idx += step_result.offset;
			if (step_result.branch != std::nullopt) {
				sub_instr.branch(step_result.branch->is_default, line);
				ScanRoutineResult scan_result = scan_routine(lines, idx + 1, agent, sub_instr);
				if (scan_result.err != std::nullopt) {
					return { .err = scan_result.err };
				}
			}
			continue;
        }
        return { .offset = idx + result.offset };
    }
    return { .offset = end };
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

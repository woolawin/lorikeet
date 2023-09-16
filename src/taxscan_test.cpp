#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <optional>
#include "taxscan.h"

class TestAgent: public Agent {
    public:
    StepResult step(const InstructionTaxonomy& instr, Peek& peek) {
        if (instr.name == "hexdump") {
            for (size_t count = 0; true; count++) {
                const std::optional<Line> line = peek.peek();
//                if (line == std::nullopt) {
//                    return StepResult{
//                        err: &TaxScanError{
//                            message:    "instruction `hexdump` not terminated with 'end' line",
//                            isEofError: true,
//                        },
//                    }
//                }
                if (line->only_non_whitespace_equals("end")) {
                    return { .offset = count + 1, .input_count = count };
                }
            }
            return {};
        }
        if (instr.name == "if") {
            return { .branch = (BranchResult){ .is_default = true } };
        }
        if (instr.name == "oops") {
            return { .branch = (BranchResult){ .is_default = true } };
        }
        return {};
    }

    SubroutineResult subroutine(const InstructionTaxonomy& instr, const Line& line) {
//        if (instr.name == "oops") {
//            return SubroutineResult{err: &TaxScanError{message: "some error"}};
//        }
        if (instr.name == "if" && line.only_non_whitespace_equals("}")) {
            return { .add = false, .step = false, .offset = 0};
        }
        if (instr.name == "if" && line.is_seq_of_strings({"}", "else", "{"})) {
            return { .add = false, .step = false, .branch = (BranchResult){}, .offset = 0};
        }
        return {. add = true, .step = true};
    }
};

TestAgent agent = TestAgent();

TEST(TaxScan, TestScanLinesOneByOne) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"var name := 'bob'",
		"debug()",
		"exit",
	};

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse("print 'Hello'")},
					.branches = {}
				},
				{
					.name =     "var",
					.input =    {parse("var name := 'bob'")},
					.branches = {}
				},
				{
					.name =     "debug",
					.input =    {parse("debug()")},
					.branches = {}
				},
				{
					.name =     "exit",
					.input =    {parse("exit")},
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}
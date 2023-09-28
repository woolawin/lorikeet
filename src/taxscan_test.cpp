#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <optional>
#include "taxscan.h"

const InstructionID INSTR_ID_NOOP     = 1;
const InstructionID INSTR_ID_IF       = 2;
const InstructionID INSTR_ID_CURL     = 3;
const InstructionID INSTR_ID_HEXDUMP  = 4;
const InstructionID INSTR_ID_VAR      = 5;
const InstructionID INSTR_ID_STDOUT   = 6;
const InstructionID INSTR_ID_DEBUG    = 7;
const InstructionID INSTR_ID_EXIT     = 8;
const InstructionID INSTR_ID_PRINT    = 9;

class TestStateMachine: public StateMachine {
    public:
    std::optional<InstructionID> find_instr(const std::string& name) {
        if (name == "noop") {
            return INSTR_ID_NOOP;
        }
        if (name == "if") {
            return INSTR_ID_IF;
        }
        if (name == "curl") {
            return INSTR_ID_CURL;
        }
        if (name == "hexdump") {
            return INSTR_ID_HEXDUMP;
        }
        if (name == "var") {
            return INSTR_ID_VAR;
        }
        if (name == "stdout") {
            return INSTR_ID_STDOUT;
        }
        if (name == "debug") {
            return INSTR_ID_DEBUG;
        }
        if (name == "exit") {
            return INSTR_ID_EXIT;
        }
        if (name == "print") {
            return INSTR_ID_PRINT;
        }
        return std::nullopt;
    }

    TaxStrat tax_strat(InstructionID instr) {
        switch (instr) {
        case INSTR_ID_NOOP:
            return value_strat();
        case INSTR_ID_IF:
            return branch_strat({"else"});
        case INSTR_ID_CURL:
            return command_strat();
        case INSTR_ID_HEXDUMP:
            return custom_strat(BlockFunction::Append);
        default:
            return command_strat();
        }
    }
};

TestStateMachine machine = TestStateMachine();

TEST(TaxScan, SingleInstruction) {
	std::vector<std::string> lines = {
		"stdout 'Hello'",
	};

	FileTaxonomy actual  = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_STDOUT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, InstructionWithNABlockFunctionCanNotHaveBlock) {
	std::vector<std::string> lines = {
	    "noop",
		"   stdout 'Hello'"
	};

	FileTaxonomy actual  = scan_file(lines, machine);

	FileTaxonomy expected = {
		.errors = {
		    instruction_does_not_accept_block(2)
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, SingleInstructionWithDirectQuotes) {
	std::vector<std::string> lines = {
		"`stdout` 'Hello'",
	};

	FileTaxonomy actual  = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_STDOUT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanLinesOneByOne) {
	std::vector<std::string> lines = {
		"stdout 'Hello'",
		"var name := 'bob'",
		"debug()",
		"exit",
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_STDOUT,
					.branches = {}
				},
				{
					.name =     "var",
					.input =    {parse(2, " name := 'bob'")},
					.instr_id = INSTR_ID_VAR,
					.branches = {}
				},
				{
					.name =     "debug",
					.input =    {parse(3, "()")},
					.instr_id = INSTR_ID_DEBUG,
					.branches = {}
				},
				{
					.name =     "exit",
					.input =    {parse(4, "")},
					.instr_id = INSTR_ID_EXIT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithInputBlockAndEndTerminator) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"hexdump",
		"	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"end"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.instr_id = INSTR_ID_HEXDUMP,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithInputBlockWithoutEndTerminator) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"hexdump",
		"	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"print 'done'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.instr_id = INSTR_ID_HEXDUMP,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(6, " 'done'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithInputBlockIgnoresComment) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"hexdump",
		"	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"# ignore me",
		"   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	# and me",
		"	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"print 'done'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(5, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(7, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.instr_id = INSTR_ID_HEXDUMP,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(8, " 'done'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanInputBlockWithEndTerminatorAndInstructionAfter) {
	std::vector<std::string> lines = {
		"print 'start'",
		"hexdump",
		"	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"	0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"end",
		"print 'done'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'start'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.instr_id = INSTR_ID_HEXDUMP,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(7, " 'done'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {},
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanIgnoresComments) {
	std::vector<std::string> lines = {
		"print 'A'",
		"# print 'B'",
		"print 'C'",
		"#< print 'D'",
		"print 'E'",
		"print 'F' >#",
		"print 'G'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'A'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(3, " 'C'")},
					.instr_id = INSTR_ID_PRINT,
					.branches   {}
				},
				{
					.name =     "print",
					.input =    {parse(7, " 'G'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithAppendToSingleLineInput) {
	std::vector<std::string> lines = {
		"curl -X POST",
		"   http://foo/bar",
		"   --header Authorization: hello",
		"   --body 'baz'",
		"print 'done'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name = "curl",
					.input = {
						parse(1, " -X POST"),
						parse(2, "http://foo/bar"),
						parse(3, "--header Authorization: hello"),
						parse(4, "--body 'baz'")
					},
					.instr_id = INSTR_ID_CURL,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(5, " 'done'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {},
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithSubroutine) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		""
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithWithBranchThatHasInput) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"else if false",
		"   print 'Bye'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(4, " if false"),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =     {parse(5, " 'Bye'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithMultipleBranches) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"else",
		"   print 'Bye'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(4, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =     {parse(5, " 'Bye'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithMultipleBranchesWithMultipleInstructions) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"	print 'Earth'",
		"else",
		"   print 'Bye'",
		"   print 'Cheerio'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(4, " 'Earth'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(5, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =     {parse(6, " 'Bye'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =     "print",
										.input =     {parse(7, " 'Cheerio'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithMultipleBranchesIgnoreComments) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"#	print 'Earth'",
		"else",
		"#   print 'Bye'",
		"   print 'Cheerio'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(5, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =     {parse(7, " 'Cheerio'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanWithNestedSubroutine) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"   print 'World'",
		"   if say_goodbye",
		"       print 'Bye'",
		""
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = RoutineTaxonomy{
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.instr_id = INSTR_ID_IF,
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.statements = {
														{
															.name =     "print",
															.input =    {parse(5, " 'Bye'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanInstructionNestedSubroutines) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"	if say_goodbye",
		"		print 'Bye'",
		"	print 'done'",
		"print 'exit'"
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.instr_id = INSTR_ID_IF,
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.statements = {
														{
															.name =     "print",
															.input =    {parse(5, " 'Bye'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse(6, " 'done'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				},
				{
					.name =     "print",
					.input =    {parse(7, " 'exit'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanSeriesOfSubroutineAnd3LevelNestWithMultipleInstructions) {
	std::vector<std::string> lines = {
		"print 'A'",
		"if true",
		"	print 'B'",
		"	if say_goodbye",
		"		print 'C'",
		"		print 'D'",
		"		if yes",
		"			print 'D-1'",
		"			print 'D-2'",
		"",
		"		print 'E'",
		"	",
		"	print 'F'",
		"	print 'G'",
		"	if !false",
		"		print 'H'",
		"		print 'I'",
		"	",
		"	print 'J'",
		"	print 'K'",
		"",
		"print 'L'",
		"print 'M'",
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'A'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {},
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name =     "print",
										.input =    {parse(3, " 'B'")},
										.instr_id = INSTR_ID_PRINT,
										.branches  {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.instr_id = INSTR_ID_IF,
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.statements = {
														{
															.name =     "print",
															.input =    {parse(5, " 'C'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse(6, " 'D'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														},
														{
															.name =  "if",
															.input = {parse(7, " yes")},
															.instr_id = INSTR_ID_IF,
															.branches = {
																{
																	.default_branch = true,
																	.input =         parse(0, ""),
																	.routine = {
																		.statements = {
																			{
																				.name =     "print",
																				.input =    {parse(8, " 'D-1'")},
																				.instr_id = INSTR_ID_PRINT,
																				.branches = {}
																			},
																			{
																				.name =     "print",
																				.input =    {parse(9, " 'D-2'")},
																				.instr_id = INSTR_ID_PRINT,
																				.branches = {}
																			}
																		}
																	}
																}
															}
														},
														{
															.name =     "print",
															.input =    {parse(11, " 'E'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse(13, " 'F'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(14, " 'G'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(15, " !false")},
										.instr_id = INSTR_ID_IF,
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.statements = {
														{
															.name =     "print",
															.input =    {parse(16, " 'H'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse(17, " 'I'")},
															.instr_id = INSTR_ID_PRINT,
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse(19, " 'J'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(20, " 'K'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				},
				{
					.name =     "print",
					.input =    {parse(22, " 'L'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(23, " 'M'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, ScanAppendWithinSubroutine) {
	std::vector<std::string> lines = {
		"print 'start'",
		"if true",
		"	hexdump",
		"		0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"		0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"		0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	end",
		"	print 'done'",
		""
	};

	FileTaxonomy actual = scan_file(lines, machine);

	FileTaxonomy expected = {
		.routine = {
			.statements = {
				{
					.name =     "print",
					.input =    {parse(1, " 'start'")},
					.instr_id = INSTR_ID_PRINT,
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.instr_id = INSTR_ID_IF,
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.statements = {
									{
										.name = "hexdump",
										.input = {
											parse(4, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
											parse(5, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
											parse(6, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
										},
										.instr_id = INSTR_ID_HEXDUMP,
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(8, " 'done'")},
										.instr_id = INSTR_ID_PRINT,
										.branches = {}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	EXPECT_EQ(actual, expected);
}

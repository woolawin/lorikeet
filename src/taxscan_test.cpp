#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <optional>
#include "taxscan.h"

class TestAgent: public Agent {
    public:
    TaxStrat tax_strat(const std::string& name) {
        if (name == "noop") {
            return value_strat();
        }
        if (name == "if") {
            return branch_strat({"else"});
        }
        if (name == "curl") {
            return command_strat();
        }
        if (name == "hexdump") {
            return custom_strat(BlockFunction::Append);
        }
        return command_strat();
    }
};

TestAgent agent = TestAgent();

TEST(TaxScan, SingleInstruction) {
	std::vector<std::string> lines = {
		"stdout 'Hello'",
	};

	FileTaxonomy actual  = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
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

	FileTaxonomy actual  = scan_file(lines, agent);

	FileTaxonomy expected = {
		.errors = {
		    instruction_does_not_accept_block()
		}
	};

	EXPECT_EQ(actual, expected);
}


TEST(TaxScan, SingleInstructionWithDirectQuotes) {
	std::vector<std::string> lines = {
		"`stdout` 'Hello'",
	};

	FileTaxonomy actual  = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "stdout",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =     "var",
					.input =    {parse(2, " name := 'bob'")},
					.branches = {}
				},
				{
					.name =     "debug",
					.input =    {parse(3, "()")},
					.branches = {}
				},
				{
					.name =     "exit",
					.input =    {parse(4, "")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(6, " 'done'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(5, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(7, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(8, " 'done'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'start'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse(3, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse(4, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse(5, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(7, " 'done'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'A'")},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(3, " 'C'")},
					.branches   {}
				},
				{
					.name =     "print",
					.input =    {parse(7, " 'G'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name = "curl",
					.input = {
						parse(1, " -X POST"),
						parse(2, "http://foo/bar"),
						parse(3, "--header Authorization: hello"),
						parse(4, "--body 'baz'")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(5, " 'done'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(4, " if false"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =     {parse(5, " 'Bye'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(4, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =     {parse(5, " 'Bye'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(4, " 'Earth'")},
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(5, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =     {parse(6, " 'Bye'")},
										.branches = {}
									},
									{
										.name =     "print",
										.input =     {parse(7, " 'Cheerio'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse(5, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =     {parse(7, " 'Cheerio'")},
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
		"	if say_goodbye",
		"       print 'Bye'",
		""
	};

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = RoutineTaxonomy{
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse(5, " 'Bye'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'World'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse(5, " 'Bye'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'A'")},
					.branches = {},
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(3, " 'B'")},
										.branches  {}
									},
									{
										.name =  "if",
										.input = {parse(4, " say_goodbye")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse(5, " 'C'")},
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse(6, " 'D'")},
															.branches = {}
														},
														{
															.name =  "if",
															.input = {parse(7, " yes")},
															.branches = {
																{
																	.default_branch = true,
																	.input =         parse(0, ""),
																	.routine = {
																		.instructions = {
																			{
																				.name =     "print",
																				.input =    {parse(8, " 'D-1'")},
																				.branches = {}
																			},
																			{
																				.name =     "print",
																				.input =    {parse(9, " 'D-2'")},
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
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(14, " 'G'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse(15, " !false")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse(0, ""),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse(16, " 'H'")},
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse(17, " 'I'")},
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
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse(20, " 'K'")},
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
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(23, " 'M'")},
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

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse(1, " 'start'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(2, " true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(0, ""),
							.routine = {
								.instructions = {
									{
										.name = "hexdump",
										.input = {
											parse(4, "0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
											parse(5, "0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
											parse(6, "0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
										},
										branches: {}
									},
									{
										.name =     "print",
										.input =    {parse(8, " 'done'")},
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

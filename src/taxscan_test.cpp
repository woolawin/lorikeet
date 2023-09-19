#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <optional>
#include "taxscan.h"

class TestAgent: public Agent {
    public:
    TaxStrat tax_strat(const std::string& name) {
        if (name == "if") {
            return { .block_kind = ROUTINE };
        }
        if (name == "curl") {
            return { .block_kind = APPEND };
        }
        if (name == "hexdump") {
            return { .block_kind = INPUT };
        }
        return { .block_kind = NA };
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
					.input =    {parse(" 'Hello'")},
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
					.input =    {parse(" 'Hello'")},
					.branches = {}
				},
				{
					.name =     "var",
					.input =    {parse(" name := 'bob'")},
					.branches = {}
				},
				{
					.name =     "debug",
					.input =    {parse("()")},
					.branches = {}
				},
				{
					.name =     "exit",
					.input =    {parse("")},
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
					.input =    {parse(" 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse("	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse("   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse("	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
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
					.input =    {parse(" 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse("	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse("   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse("	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'done'")},
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
					.input =    {parse(" 'Hello'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse("	0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse("   0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse("	0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'done'")},
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
					.input =    {parse(" 'start'")},
					.branches = {}
				},
				{
					.name = "hexdump",
					.input = {
						parse("\t0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
						parse("\t0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
						parse("\t0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'done'")},
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
					.input =    {parse(" 'A'")},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'C'")},
					.branches   {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'G'")},
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
						parse(" -X POST http://foo/bar --header Authorization: hello --body 'baz'")
					},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse(" 'done'")},
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
					.input =    {parse(" 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse(" true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse(""),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse(" 'World'")},
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
/*
TEST(TaxScan, ScanWithMultipleBranches) {
	std::vector<std::string> lines = {
		"print 'Hello'",
		"if true",
		"	print 'World'",
		"} else {",
		"   print 'Bye'",
		"}"
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
					.name =  "if",
					.input = {parse("if true")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse("if true"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse("\tprint 'World'")},
										.branches = {}
									}
								}
							}
						},
						{
							.default_branch = false,
							.input =         parse("} else {"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =     {parse("   print 'Bye'")},
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
		"if true {",
		"	print 'World'",
		"	if say_goodbye {",
		"		print 'Bye'",
		"	}",
		"}"
	};

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = RoutineTaxonomy{
			.instructions = {
				{
					.name =     "print",
					.input =    {parse("print 'Hello'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse("if true {")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse("if true {"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse("\tprint 'World'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse("\tif say_goodbye {")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse("\tif say_goodbye {"),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'Bye'")},
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
		"if true {",
		"	print 'World'",
		"	if say_goodbye {",
		"		print 'Bye'",
		"	}",
		"	print 'done'",
		"}",
		"print 'exit'"
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
					.name =  "if",
					.input = {parse("if true {")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse("if true {"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse("\tprint 'World'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse("\tif say_goodbye {")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse("\tif say_goodbye {"),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'Bye'")},
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'done'")},
										.branches = {}
									}
								}
							}
						}
					}
				},
				{
					.name =     "print",
					.input =    {parse("print 'exit'")},
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
		"if true {",
		"	print 'B'",
		"	if say_goodbye {",
		"		print 'C'",
		"		print 'D'",
		"		if yes {",
		"			print 'D-1'",
		"			print 'D-2'",
		"		}",
		"		print 'E'",
		"	}",
		"	print 'F'",
		"	print 'G'",
		"	if !false {",
		"		print 'H'",
		"		print 'I'",
		"	}",
		"	print 'J'",
		"	print 'K'",
		"}",
		"print 'L'",
		"print 'M'",
	};

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse("print 'A'")},
					.branches = {},
				},
				{
					.name =  "if",
					.input = {parse("if true {")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse("if true {"),
							.routine = {
								.instructions = {
									{
										.name =     "print",
										.input =    {parse("\tprint 'B'")},
										.branches  {}
									},
									{
										.name =  "if",
										.input = {parse("\tif say_goodbye {")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse("\tif say_goodbye {"),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'C'")},
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'D'")},
															.branches = {}
														},
														{
															.name =  "if",
															.input = {parse("\t\tif yes {")},
															.branches = {
																{
																	.default_branch = true,
																	.input =         parse("\t\tif yes {"),
																	.routine = {
																		.instructions = {
																			{
																				.name =     "print",
																				.input =    {parse("\t\t\tprint 'D-1'")},
																				.branches = {}
																			},
																			{
																				.name =     "print",
																				.input =    {parse("\t\t\tprint 'D-2'")},
																				.branches = {}
																			}
																		}
																	}
																}
															}
														},
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'E'")},
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'F'")},
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'G'")},
										.branches = {}
									},
									{
										.name =  "if",
										.input = {parse("\tif !false {")},
										.branches = {
											{
												.default_branch = true,
												.input =         parse("\tif !false {"),
												.routine = {
													.instructions = {
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'H'")},
															.branches = {}
														},
														{
															.name =     "print",
															.input =    {parse("\t\tprint 'I'")},
															.branches = {}
														}
													}
												}
											}
										}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'J'")},
										.branches = {}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'K'")},
										.branches = {}
									}
								}
							}
						}
					}
				},
				{
					.name =     "print",
					.input =    {parse("print 'L'")},
					.branches = {}
				},
				{
					.name =     "print",
					.input =    {parse("print 'M'")},
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
		"if true {",
		"	hexdump",
		"		0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46",
		"		0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"		0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a",
		"	end",
		"	print 'done'",
		"}"
	};

	FileTaxonomy actual = scan_file(lines, agent);

	FileTaxonomy expected = {
		.routine = {
			.instructions = {
				{
					.name =     "print",
					.input =    {parse("print 'start'")},
					.branches = {}
				},
				{
					.name =  "if",
					.input = {parse("if true {")},
					.branches = {
						{
							.default_branch = true,
							.input =         parse("if true {"),
							.routine = {
								.instructions = {
									{
										.name = "hexdump",
										.input = {
											parse("	hexdump"),
											parse("\t\t0000000 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46"),
											parse("\t\t0000010 0a 2f 2a 20 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
											parse("\t\t0000020 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a"),
										},
										branches: {}
									},
									{
										.name =     "print",
										.input =    {parse("\tprint 'done'")},
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
*/
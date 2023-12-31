#include <gtest/gtest.h>

#include "line.h"

TEST(Line, TwoWordsWithWhitespaceBetween) {
    Line actual = parse(1, "print data");
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            word_token("print"),
            whitespace_token(" "),
            word_token("data")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, BackquotesMakesWords) {
    Line actual = parse(1, "`clang++` { `}`");
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            word_token("clang++"),
            whitespace_token(" "),
            symbol_token("{"),
            whitespace_token(" "),
            word_token("}")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, WithWhitespaceAtStartAndEnd) {
    Line actual = parse(1, " \t  \t print data\t\t  ");
    Line expected = {
        .line_num = 1,
        .start = 1,
        .end = 3,
        .word_start = 1,
        .tokens = {
            whitespace_token(" \t  \t "),
            word_token("print"),
            whitespace_token(" "),
            word_token("data"),
            whitespace_token("\t\t  ")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, SymbolAtEndOfLine) {
    Line actual = parse(1, "if true {");
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            word_token("if"),
            whitespace_token(" "),
            word_token("true"),
            whitespace_token(" "),
            symbol_token("{")
        }
    };

    EXPECT_EQ(actual, expected);
}


TEST(Line, Quotes) {
    Line actual = parse(1, "if 'yes'");
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            word_token("if"),
            whitespace_token(" "),
            symbol_token("'"),
            word_token("yes"),
            symbol_token("'")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, SymbolsInMiddleWithoutSpaces) {
    Line actual = parse(1, "count+=1");
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 3,
        .word_start = 0,
        .tokens = {
            word_token("count"),
            symbol_token("+"),
            symbol_token("="),
            word_token("1")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, WhitespaceSymbolWhitespaceThenWord) {
    Line actual = parse(1, "\t| echo");
    Line expected = {
        .line_num = 1,
        .start = 1,
        .end = 3,
        .word_start = 3,
        .tokens = {
            whitespace_token("\t"),
            symbol_token("|"),
            whitespace_token(" "),
            word_token("echo")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, HasSymbolSeqNoSymbols) {
	std::string line = "foo bar";
	EXPECT_FALSE(parse(1, line).has_symbol_seq(0, "=>"));
}

TEST(Line, HasSymbolHasEq) {
	std::string line = "foo=>bar";
	EXPECT_TRUE(parse(1, line).has_symbol_seq(1, "=>"));
}

TEST(Line, HasSymbolInsufficientLength) {
	std::string line = "foo:=";
	EXPECT_FALSE(parse(1, line).has_symbol_seq(1, ":=>"));
}

TEST(Line, HasSymbolAtEnd) {
	std::string line = "foo ||";
	EXPECT_TRUE(parse(1, line).has_symbol_seq(2, "||"));
}

TEST(Line, LineEndsWithJustSymbol) {
	std::string line = "}";
	EXPECT_TRUE(parse(1, line).ends_with_symbol_seq("}"));
}

TEST(Line, EndsWithSymbolSeqWithWordBeforeSymbolSeq) {
	std::string line = " foo }";
	EXPECT_TRUE(parse(1, line).ends_with_symbol_seq("}"));
}

TEST(Line, EndsWithSymbolSeqWithMultiCharacterSymbol) {
	std::string line = "blah blah >#";
	EXPECT_TRUE(parse(1, line).ends_with_symbol_seq(">#"));
}

TEST(Line, LineDoesNotEndWithSymbol) {
	std::string line = "blah blah ># fff";
	EXPECT_FALSE(parse(1, line).ends_with_symbol_seq(">#"));
}

TEST(Line, IsSeqOfStrings) {
	std::string line = "} else {";
	EXPECT_TRUE(parse(1, line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsSeqOfStringsIgnoringWhitespace) {
	std::string line = "\t\t\t   }   \telse   \t \t  {  ";
	EXPECT_TRUE(parse(1, line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsNotSeqOfStringsIfEndsWithMoreTokens) {
	std::string line = "} else { then";
	EXPECT_FALSE(parse(1, line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsNotSeqOfStringsIfStartsWithDifferentToken) {
	std::string line = "do else {";
	EXPECT_FALSE(parse(1, line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, MinusFirstWordWhenFirstTokenIsWord) {
    Line actual = parse(1, "foo bar baz");
    Line expected = parse(1, " bar baz");
    EXPECT_EQ(actual.crop_from_first_word(), expected);
}

TEST(Line, MinusFirstWordWhenFirstTokensAreWhitespace) {
    Line actual = parse(1, "  \t  \tfoo.doh doh\t");
    Line expected = parse(1, ".doh doh\t");
    EXPECT_EQ(actual.crop_from_first_word(), expected);
}

TEST(Indentation, SameIndentationWithNoLevels) {
    Indentation indentation = { .indentations = {} };
    EXPECT_EQ(indentation.diff(""), IndentationDiff::Same);
}

TEST(Indentation, FirstIndentationLevel) {
    Indentation indentation = { .indentations = {} };
    EXPECT_EQ(indentation.diff(" "), IndentationDiff::Increase);
}

TEST(Indentation, FirstLevelOfIndentation) {
    Indentation indentation = { .indentations = { "" } };
    EXPECT_EQ(indentation.diff(" "), IndentationDiff::Increase);
}

TEST(Indentation, SecondLevelOfIndentation) {
    Indentation indentation = { .indentations = { "", "  " } };
    EXPECT_EQ(indentation.diff("    "), IndentationDiff::Increase);
}

TEST(Indentation, SameIndentationLevelOnNonFirstLevel) {
    Indentation indentation = { .indentations = { "", "  " } };
    EXPECT_EQ(indentation.diff("  "), IndentationDiff::Same);
}

TEST(Indentation, IndentationWentBackOneLevel) {
    Indentation indentation = { .indentations = { "", "  " } };
    EXPECT_EQ(indentation.diff(""), IndentationDiff::Decrease);
}

TEST(Indentation, IndentationWentBackOneLevelButNotToFirstLevel) {
    Indentation indentation = { .indentations = { "", " ", "  ", "   ", "    " } };
    EXPECT_EQ(indentation.diff("  "), IndentationDiff::Decrease);
}

TEST(Indentation, WrongIndentation) {
    Indentation indentation = { .indentations = { "", " ", "  " } };
    EXPECT_EQ(indentation.diff(" \t"), IndentationDiff::Error);
}

TEST(Indentation, WrongIndentation2) {
    Indentation indentation = { .indentations = { "", " ", "  ", "   " } };
    EXPECT_EQ(indentation.diff(" \t "), IndentationDiff::Error);
}


TEST(Line, parse_quotes_with_single_quote) {
    Line actual = parse_quotes(parse(1, "stdout 'Hello World'"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            word_token("stdout"),
            whitespace_token(" "),
            quote_token("Hello World", "'")
        }
    };

    EXPECT_EQ(actual, expected);
}


TEST(Line, parse_quotes_with_double_quote) {
    Line actual = parse_quotes(parse(1, "stdout \"Hello World\""));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            word_token("stdout"),
            whitespace_token(" "),
            quote_token("Hello World", "\"")
        }
    };

    EXPECT_EQ(actual, expected);
}


TEST(Line, parse_quotes_escapes_single_quote) {
    Line actual = parse_quotes(parse(1, "stdout 'Hello W\\'orld'"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            word_token("stdout"),
            whitespace_token(" "),
            quote_token("Hello W'orld", "'")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_quotes_escapes_double_quote) {
    Line actual = parse_quotes(parse(1, "stdout \"Hello W\\\"orld\""));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            word_token("stdout"),
            whitespace_token(" "),
            quote_token("Hello W\"orld", "\"")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_quotes_multiple_quotes) {
    Line actual = parse_quotes(parse(1, "stdout \"Hello \" 'W o r l d'"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            word_token("stdout"),
            whitespace_token(" "),
            quote_token("Hello ", "\""),
            whitespace_token(" "),
            quote_token("W o r l d", "'")
        }
    };

    EXPECT_EQ(actual, expected);
}


TEST(Line, parse_flags_short_flag) {
    Line actual = parse_flags(parse(1, "ping -c 1 foo"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 6,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("c", "-"),
            whitespace_token(" "),
            word_token("1"),
            whitespace_token(" "),
            word_token("foo")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_flags_long_flags) {
    Line actual = parse_flags(parse(1, "ping --count 1 foo"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 6,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("count", "--"),
            whitespace_token(" "),
            word_token("1"),
            whitespace_token(" "),
            word_token("foo")
        }
    };

    EXPECT_EQ(actual, expected);
}


TEST(Line, parse_flags_long_flag_that_contains_dash) {
    Line actual = parse_flags(parse(1, "ping --co-unt 1 foo"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 6,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("co-unt", "--"),
            whitespace_token(" "),
            word_token("1"),
            whitespace_token(" "),
            word_token("foo")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_flags_with_multiple) {
    Line actual = parse_flags(parse(1, "ping -d --count 1 -fc foo --out file"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 14,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("d", "-"),
            whitespace_token(" "),
            flag_token("count", "--"),
            whitespace_token(" "),
            word_token("1"),
            whitespace_token(" "),
            flag_token("fc", "-"),
            whitespace_token(" "),
            word_token("foo"),
            whitespace_token(" "),
            flag_token("out", "--"),
            whitespace_token(" "),
            word_token("file")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_flags_with_parse_quotes) {
    Line actual = parse_flags(parse_quotes(parse(1, "ping --count 1 'foo'")));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 6,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("count", "--"),
            whitespace_token(" "),
            word_token("1"),
            whitespace_token(" "),
            quote_token("foo", "'")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_flags_two_dashes_without_word_is_not_flag) {
    Line actual = parse_flags(parse(1, "ping -- foo"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 5,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            symbol_token("-"),
            symbol_token("-"),
            whitespace_token(" "),
            word_token("foo")
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, parse_flags_with_quoted_flag) {
    Line actual = parse_flags(parse(1, "ping --`c o u n t` foo"));
    Line expected = {
        .line_num = 1,
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            word_token("ping"),
            whitespace_token(" "),
            flag_token("c o u n t", "--"),
            whitespace_token(" "),
            word_token("foo")
        }
    };

    EXPECT_EQ(actual, expected);
}
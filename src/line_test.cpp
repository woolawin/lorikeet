#include <gtest/gtest.h>

#include "line.h"

TEST(Line, TwoWordsWithWhitespaceBetween) {
    Line actual = parse("print data");
    Line expected = {
        .start = 0,
        .end = 2,
        .word_start = 0,
        .tokens = {
            {
                .kind = WORD,
                .value = "print"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = WORD,
                .value = "data"
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, BackquotesMakesWords) {
    Line actual = parse("`clang++` { `}`");
    Line expected = {
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            {
                .kind = WORD,
                .value = "clang++"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = SYMBOL,
                .value = "{"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = WORD,
                .value = "}"
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, WithWhitespaceAtStartAndEnd) {
    Line actual = parse(" \t  \t print data\t\t  ");
    Line expected = {
        .start = 1,
        .end = 3,
        .word_start = 1,
        .tokens = {
            {
                .kind = WHITESPACE,
                .value = " \t  \t "
            },
            {
                .kind = WORD,
                .value = "print"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = WORD,
                .value = "data"
            },
            {
                .kind = WHITESPACE,
                .value = "\t\t  "
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, SymbolAtEndOfLine) {
    Line actual = parse("if true {");
    Line expected = {
        .start = 0,
        .end = 4,
        .word_start = 0,
        .tokens = {
            {
                .kind = WORD,
                .value = "if"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = WORD,
                .value = "true"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = SYMBOL,
                .value = "{"
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, SymbolsInMiddleWithoutSpaces) {
    Line actual = parse("count+=1");
    Line expected = {
        .start = 0,
        .end = 3,
        .word_start = 0,
        .tokens = {
            {
                .kind = WORD,
                .value = "count"
            },
            {
                .kind = SYMBOL,
                .value = "+"
            },
            {
                .kind = SYMBOL,
                .value = "="
            },
            {
                .kind = WORD,
                .value = "1"
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, WhitespaceSymbolWhitespaceThenWord) {
    Line actual = parse("\t| echo");
    Line expected = {
        .start = 1,
        .end = 3,
        .word_start = 3,
        .tokens = {
            {
                .kind = WHITESPACE,
                .value = "\t"
            },
            {
                .kind = SYMBOL,
                .value = "|"
            },
            {
                .kind = WHITESPACE,
                .value = " "
            },
            {
                .kind = WORD,
                .value = "echo"
            }
        }
    };

    EXPECT_EQ(actual, expected);
}

TEST(Line, HasSymbolSeqNoSymbols) {
	std::string line = "foo bar";
	EXPECT_FALSE(parse(line).has_symbol_seq(0, "=>"));
}

TEST(Line, HasSymbolHasEq) {
	std::string line = "foo=>bar";
	EXPECT_TRUE(parse(line).has_symbol_seq(1, "=>"));
}

TEST(Line, HasSymbolInsufficientLength) {
	std::string line = "foo:=";
	EXPECT_FALSE(parse(line).has_symbol_seq(1, ":=>"));
}

TEST(Line, HasSymbolAtEnd) {
	std::string line = "foo ||";
	EXPECT_TRUE(parse(line).has_symbol_seq(2, "||"));
}

TEST(Line, LineEndsWithJustSymbol) {
	std::string line = "}";
	EXPECT_TRUE(parse(line).ends_with_symbol_seq("}"));
}

TEST(Line, EndsWithSymbolSeqWithWordBeforeSymbolSeq) {
	std::string line = " foo }";
	EXPECT_TRUE(parse(line).ends_with_symbol_seq("}"));
}

TEST(Line, EndsWithSymbolSeqWithMultiCharacterSymbol) {
	std::string line = "blah blah >#";
	EXPECT_TRUE(parse(line).ends_with_symbol_seq(">#"));
}

TEST(Line, LineDoesNotEndWithSymbol) {
	std::string line = "blah blah ># fff";
	EXPECT_FALSE(parse(line).ends_with_symbol_seq(">#"));
}

TEST(Line, IsSeqOfStrings) {
	std::string line = "} else {";
	EXPECT_TRUE(parse(line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsSeqOfStringsIgnoringWhitespace) {
	std::string line = "\t\t\t   }   \telse   \t \t  {  ";
	EXPECT_TRUE(parse(line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsNotSeqOfStringsIfEndsWithMoreTokens) {
	std::string line = "} else { then";
	EXPECT_FALSE(parse(line).is_seq_of_strings({"}", "else", "{"}));
}

TEST(Line, IsNotSeqOfStringsIfStartsWithDifferentToken) {
	std::string line = "do else {";
	EXPECT_FALSE(parse(line).is_seq_of_strings({"}", "else", "{"}));
}
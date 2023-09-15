#include <gtest/gtest.h>

#include "taxscan.h"

TEST(Reader, TwoWordsWithWhitespaceBetween) {
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

TEST(Reader, BackquotesMakesWords) {
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

TEST(Reader, WithWhitespaceAtStartAndEnd) {
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

TEST(Reader, SymbolAtEndOfLine) {
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

TEST(Reader, SymbolsInMiddleWithoutSpaces) {
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

TEST(Reader, WhitespaceSymbolWhitespaceThenWord) {
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

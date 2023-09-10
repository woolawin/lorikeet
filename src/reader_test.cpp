#include <gtest/gtest.h>

#include "taxscan.h"

bool linesEqual(const Line& expected, const Line& actual) {
    bool start_and_end_is_equal =  expected.start == actual.start
        && expected.end == actual.end
        && expected.word_start == actual.word_start;
    if (!start_and_end_is_equal) {
        return false;
    }
    return true;
}

TEST(TaxscanReader, First) {
    Line actual = parse("hello");
    Line expected = {.start = 0, .end = 0, .word_start = 0, .tokens = {}};
    EXPECT_TRUE(linesEqual(expected, actual));
}

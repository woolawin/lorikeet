#include <gtest/gtest.h>

#include "taxscan.h"

TEST(TaxscanReader, First) {
    Line actual = parse("hello");
    Line expected = {.start = 0, .end = 0, .word_start = 0, .tokens = {}};

    EXPECT_EQ(actual, expected);
}

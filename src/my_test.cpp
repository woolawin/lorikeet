#include <gtest/gtest.h>

TEST(MyTestCase, MyTest) {
    // Your test code here
    ASSERT_TRUE(true);
}

TEST(MyTestCase3, MyTest3) {
    // Your test code here
    ASSERT_TRUE(true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
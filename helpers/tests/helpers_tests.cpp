#include "helpers/helpers_tests.h"

TEST(Tests, Failure) {
    TEST_SKIP
    EXPECT(false);
}


TEST(Tests, ExpectWorks) {
    EXPECT(true);
    EXPECT(3, 3);
    EXPECT(3 == 3);
    EXPECT(3 == -3 * -1);
    EXPECT(3 != 4);
}

TEST(Tests, FailureEq) {
    TEST_SKIP
    EXPECT(1 + 2 + 3, 7);
}


#include "cuTest.h"

void
TestStrToUpper(CuTest *tc) {
    /**
     * Failing test.
     */
    char* actual = "Hello World";
    char* expected = "HELLO WORLD";
    CuAssertStrEquals(tc, expected, actual);
}

CuSuite* HFTLobGetSuite() {
    /**
     * Prepare the test suite.
     */
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestStrToUpper);
    return suite;
}
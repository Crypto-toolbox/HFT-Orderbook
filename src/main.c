#include <stdio.h>
#include "CuTest.h"
#include "hftlob.h"
#include "testCases.c"

CuSuite* HFTLobGetSuite();

void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, HFTLobGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(int argc, char* argv[]){
    int i;
    for(i=0; i<argc; ++i){
        if (argv[i] == '--test'){
            printf("--test flag passed, running cuTest TestSuite..", i, argv[i]);
            RunAllTests();
            return 0;
        }
    }

    Limit testLimit;
    testLimit.limitPrice = 1000;
    Limit *ptr_testLimit = testLimit;

    Order testOrderA;
    testOrderA.tid = 100;
    testOrderA.buyOrSell = 1;
    testOrderA.shares = 5;
    testOrderA.limit = 1000;

    Order *ptr_testOrderA = testOrderA;
    Order testOrderB;
    testOrderB.tid = 102;
    testOrderB.buyOrSell = 1;
    testOrderB.shares = 10;
    testOrderB.limit = 1000;

    Order *ptr_testOrderB = testOrderB;
    Order testOrderC;
    testOrderC.tid = 104;
    testOrderC.buyOrSell = 1;
    testOrderC.shares = 20;
    testOrderC.limit = 1000;
    Order *ptr_testOrderC = testOrderC;

    return 0;
}

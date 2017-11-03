#include <stdio.h>
#include "CuTest.h"
#include "hftlob.h"

int main(int argc, char* argv[]){
    int i;
    for(i=0; i<argc; ++i){
        if (argv[i] == "--test"){
            printf("--test flag passed, running cuTest TestSuite..", i, argv[i]);
            RunAllTests();
        }
    }

    return 0;
}

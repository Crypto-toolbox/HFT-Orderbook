#include <stdio.h>
#include "hftlob.h"

int main(int argc, char* argv[]){
    int i;
    for(i=0; i<argc; ++i){
        if (argv[i] == "--test"){
            printf("--test flag passed, running cuTest TestSuite..");
            RunAllTests();
        }
    }

    return 0;
}

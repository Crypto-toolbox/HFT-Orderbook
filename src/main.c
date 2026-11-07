#include <stdio.h>
#include <string.h>
#include "hftlob.h"

int main(int argc, char* argv[]){
    int i;
    for(i=1; i<argc; ++i){
        if (strcmp(argv[i], "--test") == 0){
            printf("Running CuTest test suite..\n");
            return RunAllTests() == 0 ? 0 : 1;
        }
    }
    printf("Usage: %s --test\n", argv[0]);
    return 0;
}

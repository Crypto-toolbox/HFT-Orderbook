#include <stdio.h>
#include <string.h>
#include "hftlob.h"

int main(int argc, char* argv[]){
    int i;
    printf("Running main..\n");
    for(i=0; i<argc; ++i){
        if (strcmp(argv[i], "--test") == 0){
            printf("--test flag passed, running cuTest TestSuite..\n");
            RunAllTests();
        }
    }

    return 0;
}

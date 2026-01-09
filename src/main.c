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

    // 2. Manual Simulation (The "Market" starts here)
    Book* myBook = createBook(); 

    printf("Inserting orders to trigger AVL rotations...\n");
    // Adding orders in a way that forces the tree to rebalance (100 -> 110 -> 120)
    addOrderToBook(myBook, 101, 'B', 100.00, 50);
    addOrderToBook(myBook, 102, 'B', 110.00, 50);
    addOrderToBook(myBook, 103, 'B', 120.00, 50); // This should trigger a rotation

    // 3. Visualize the results
    // This calls the function where you fixed the %d to %.2f
    printTree(myBook->bids);

    return 0;
}

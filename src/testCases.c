#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "CuTest.h"
#include "hftlob.h"


/**
 * Define some convenience functions before we start testing.
 */

/**
 * Return a Limit struct pointer
 */
Limit*
createDummyLimit(float price){
    Limit *ptr_newLimit = malloc(sizeof(Limit));
    initLimit(ptr_newLimit);
    ptr_newLimit->limitPrice = price;
    return (ptr_newLimit);
}

/**
 * Convenience function to ease setup of tests.
 */
Limit*
createDummyTree(Limit *dummyA, Limit *dummyB, Limit *dummyC, Limit *dummyD){
    int statusCode = 0;
    Limit *ptr_root = malloc(sizeof(Limit));
    ptr_root = createRoot();
    statusCode = addNewLimit(ptr_root, dummyA);
    assert(statusCode == 1);
    statusCode = addNewLimit(ptr_root, dummyB);
    assert(statusCode == 1);
    statusCode = addNewLimit(ptr_root, dummyC);
    assert(statusCode == 1);
    statusCode = addNewLimit(ptr_root, dummyD);
    assert(statusCode == 1);
    return (ptr_root);
}


/**
 * Test Functions related to adding and removing orders to/from a limit.
 */

void
TestOrderPushing(CuTest *tc){
    printf("Running TestOrderPushing()..\n");
    Limit limit;
    Limit *ptr_limit = &limit;
    initLimit(ptr_limit);
    limit.limitPrice = 1000.0;


    /**
     * Push a single order to an empty limit and check that both tail and head point towards it.
     * Also check that the inserted Order has the ptr set in its Order.parentLimit attribute.
     */
    Order newOrderA;
    Order *ptr_newOrderA = &newOrderA;
    initOrder(ptr_newOrderA);
    newOrderA.limit = 1000.0;
    newOrderA.shares = 10;
    newOrderA.buyOrSell = 0;
    newOrderA.tid = "1234";

    float expected_volume = 0.0;
    float expected_size = 0;
    int expected_orderCount = 0;
    int returnCode = 0;

    returnCode = pushOrder(ptr_limit, ptr_newOrderA);
    CuAssertIntEquals(tc, returnCode, 1);

    // Assert References have been correctly updated
    CuAssertPtrNotNull(tc, ptr_limit->headOrder);
    CuAssertPtrNotNull(tc, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_limit->headOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->parentLimit, ptr_limit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderA.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderA.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 1);
    expected_size = limit.size;
    expected_volume = limit.totalVolume;
    expected_orderCount++;


    /**
     * Push another order and assert that the ptr_limit->headOrder is updated correctly, and the orders linked together
     * properly (order.nextOrder should be NULL for tail, and order.previousOrder should be NULL for head; the other
     * pointers should point to the respective order).
     */
    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    initOrder(ptr_newOrderB);
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.tid = "1235";

    returnCode = pushOrder(ptr_limit, ptr_newOrderB);
    CuAssertIntEquals(tc, returnCode, 1);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_limit->headOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->parentLimit, ptr_limit);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->parentLimit, ptr_limit);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->nextOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder, NULL);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderB.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderB.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 2);
    expected_size = limit.size;
    expected_volume = limit.totalVolume;
    expected_orderCount++;

    /**
     * Push a final order and assert that all references are updated correctly.Make sure that not only head and tail
     * orders have their references updated correctly, but also the middle order does.
     */
    Order newOrderC;
    Order *ptr_newOrderC = &newOrderC;
    initOrder(ptr_newOrderC);
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.tid = "1236";

    returnCode = pushOrder(ptr_limit, ptr_newOrderC);
    CuAssertIntEquals(tc, returnCode, 1);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_limit->headOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->parentLimit, ptr_limit);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->parentLimit, ptr_limit);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->nextOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder->nextOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder->parentLimit, ptr_limit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderB.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderB.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 2);
    expected_size = limit.size;
    expected_volume = limit.totalVolume;
    expected_orderCount++;


    /**
     * Now push an order that does not have a matching limit and assert pushOrder return 0
     */
    Order newOrderD;
    Order *ptr_newOrderD = &newOrderD;
    initOrder(ptr_newOrderD);
    newOrderC.limit = 2000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.tid = "1236";
    returnCode = pushOrder(ptr_limit, ptr_newOrderD);
    CuAssertIntEquals(tc, returnCode, 0);

    printf("passed!\n");
}

void
TestOrderPopping(CuTest *tc){
    printf("Running TestOrderPopping()..\n");
    Limit limit;
    Limit *ptr_limit = &limit;
    initLimit(ptr_limit);
    limit.limitPrice = 1000.0;

    Order newOrderA;
    Order *ptr_newOrderA = &newOrderA;
    initOrder(ptr_newOrderA);
    newOrderA.limit = 1000.0;
    newOrderA.shares = 10;
    newOrderA.buyOrSell = 0;
    newOrderA.tid = "1234";

    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    initOrder(ptr_newOrderB);
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.tid = "1235";

    Order newOrderC;
    Order *ptr_newOrderC = &newOrderC;
    initOrder(ptr_newOrderC);
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.tid = "1236";

    pushOrder(ptr_limit, ptr_newOrderA);
    pushOrder(ptr_limit, ptr_newOrderB);
    pushOrder(ptr_limit, ptr_newOrderC);

    float currentSize = limit.size;
    float currentVolume = limit.totalVolume;
    int currentOrderCount = limit.orderCount;

    /**
     * Pop all orders and check after each pop that all references are updated correctly, along with all relevant
     * attributes in the limit.
     */

    // // // //
    // Pop # 1
    // // // //
    int isPopped = popOrder(ptr_limit);
    CuAssertIntEquals(tc, isPopped, 1);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_limit->headOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->prevOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder->nextOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->headOrder->nextOrder, ptr_newOrderB);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderA.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderA.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 2);

    // Update test vars
    currentSize = limit.size;
    currentVolume = limit.totalVolume;
    currentOrderCount = limit.orderCount;

    // // // //
    // Pop # 2
    // // // //
    isPopped = popOrder(ptr_limit);
    CuAssertIntEquals(tc, isPopped, 1);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_limit->headOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, ptr_newOrderC->prevOrder, NULL);
    CuAssertPtrEquals(tc, ptr_newOrderC->nextOrder, NULL);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderB.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderB.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 1);

    // Update test vars
    currentSize = limit.size;
    currentVolume = limit.totalVolume;
    currentOrderCount = limit.orderCount;

    // // // //
    // Pop # 3
    // // // //
    isPopped = popOrder(ptr_limit);
    CuAssertIntEquals(tc, isPopped, 1);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_limit->headOrder, NULL);
    CuAssertPtrEquals(tc, ptr_limit->tailOrder, NULL);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderC.shares * limit.limitPrice, 0.0);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderC.shares, 0.0);
    CuAssertIntEquals(tc, limit.orderCount, 0);

    // // // //
    // Pop # 4
    // // // //
    isPopped = popOrder(ptr_limit);
    CuAssertIntEquals(tc, isPopped, 0);

    printf("passed!\n");
}

/**
 * Test Root creation and tree population funcs.
 */

void
TestCreateRoot(CuTest *tc){
    printf("Running TestCreateRoot()..\n");
    /**
     * Test the createRoot() function and assert it creates a root with limitPrice of -INFINITY, no children
     * and no parent.
     */
    Limit *ptr_root = createRoot();
    CuAssertPtrEquals(tc, ptr_root->parent, NULL);
    CuAssertDblEquals(tc, ptr_root->limitPrice, -INFINITY, 0.0);
    CuAssertPtrEquals(tc, ptr_root->leftChild, NULL);
    CuAssertPtrEquals(tc, ptr_root->rightChild, NULL);

    printf("passed!\n");
}

void
TestAddNewLimit(CuTest *tc){
    printf("Running TestAddNewLimit()..\n");
    /**
     * Assert that the addNewLimit() function adds a Limit structure dependent on it limitPrice into a
     * given root Limit structure.
     */
    Limit *ptr_root = createRoot();

    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(50.0);
    Limit *ptr_newLimitC = createDummyLimit(200.0);

    int statusCode = 0;

    /**
     * Add the first limit. Assert it is added as the leftChild of root. Check its references after addition to root.
     */
    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);

    CuAssertPtrEquals(tc, ptr_root->leftChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_root->leftChild->parent, ptr_root);

    /**
      * Add the second limit. Assert it is added as root->leftChild->leftChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);

    CuAssertPtrEquals(tc, ptr_root->leftChild->leftChild, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_root->leftChild->leftChild->parent, ptr_newLimitA);

    /**
      * Add the third limit. Assert it is added as root->leftChild->rightChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);

    CuAssertPtrEquals(tc, ptr_root->leftChild->rightChild, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_root->leftChild->rightChild->parent, ptr_newLimitA);
    /**
     * Add a duplicate limit and assert the returned status code is 0.
     */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 0);

    printf("passed!\n");
}

/**
 * Test the convenience functions for their correctness.
 */

void
TestLimitExists(CuTest *tc){
    printf("Running TestLimitExists()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    /**
     * Assert that limitExists returns correct integer values.
     */
    Limit limit;
    limit.limitPrice = 100;
    Limit *ptr_limit = &limit;

    int statusCode = 0;

    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, statusCode, 1);
    limit.limitPrice = 100;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, statusCode, 1);
    limit.limitPrice = 200;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, statusCode, 1);
    limit.limitPrice = 50;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, statusCode, 45);

    printf("passed!\n");
}

void
TestLimitIsRoot(CuTest *tc){
    printf("Running TestLimitIsRoot()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    /**
     * Assert that limitIsRoot() returns the correct values.
     */

    int statusCode = 0;

    statusCode = limitIsRoot(ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 0);
    statusCode = limitIsRoot(ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 0);
    statusCode = limitIsRoot(ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 0);
    statusCode = limitIsRoot(ptr_root);
    CuAssertIntEquals(tc, statusCode, 1);

    printf("passed!\n");
}

void
TestHasGrandpa(CuTest *tc){
    printf("Running TestHasGrandpa()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);


    /**
     * Assert that hasGrandpa() returns correct values.
     */

    int statusCode = 0;

    statusCode = hasGrandpa(ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 0);
    statusCode = hasGrandpa(ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = hasGrandpa(ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = hasGrandpa(ptr_root);
    CuAssertIntEquals(tc, statusCode, 0);

    printf("passed!\n");
}

void
TestGetGrandpa(CuTest *tc){
    printf("Running TestgetGrandpa()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);


    /**
      * Assert that getGrandpa() returns a pointer if a grandpa exists and NULL otherwise.
      */

    Limit *ptr_grandpa;

    ptr_grandpa = getGrandpa(ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_grandpa, NULL);
    ptr_grandpa = getGrandpa(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_grandpa, ptr_root);
    ptr_grandpa = getGrandpa(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_grandpa, ptr_root);
    ptr_grandpa = getGrandpa(ptr_root);
    CuAssertPtrEquals(tc, ptr_grandpa, NULL);

    printf("passed!\n");
}

void
TestGetMaximumLimit(CuTest *tc){
    printf("Running TestGetMaximumLimit()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    /**
     * Assert getMaximumLimit() always returns the most-right limit present under a given limit.
     * If there is no limit under it, it returns the given limit instead.
     */

    Limit *ptr_retValue;

    ptr_retValue = getMaximumLimit(ptr_newLimitA);
    CuAssertPtrEquals(tc,ptr_retValue, ptr_newLimitC);
    ptr_retValue = getMaximumLimit(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_retValue, ptr_newLimitB);
    ptr_retValue = getMaximumLimit(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_retValue, ptr_newLimitC);
    ptr_retValue = getMaximumLimit(ptr_root);
    CuAssertPtrEquals(tc,ptr_retValue, ptr_newLimitC);

    printf("passed!\n");
}

void
TestGetMinimumLimit(CuTest *tc){
    printf("Running TestGetMinimumLimit()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    /**
     * Assert getMinimumLimit() always returns the most-right limit present under a given limit.
     * If there is no limit under it, it returns the given limit instead.
     */

    Limit *ptr_retValue;

    ptr_retValue = getMinimumLimit(ptr_newLimitA);
    CuAssertPtrEquals(tc,ptr_retValue, ptr_newLimitB);
    ptr_retValue = getMinimumLimit(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_retValue, ptr_newLimitB);
    ptr_retValue = getMinimumLimit(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_retValue, ptr_newLimitC);
    ptr_retValue = getMinimumLimit(ptr_root);
    CuAssertPtrEquals(tc,ptr_retValue, ptr_newLimitB);

    printf("passed!\n");
}

void
TestGetHeight(CuTest *tc){
    printf("Running TestGetHeight()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);


    /**
     * Assert that the height is calculated correctly.
     */

    int height = 0;

    height = getHeight(ptr_root);
    CuAssertIntEquals(tc, height, 3);
    height = getHeight(ptr_newLimitA);
    CuAssertIntEquals(tc, height, 2);
    height = getHeight(ptr_newLimitB);
    CuAssertIntEquals(tc, height, 1);
    height = getHeight(ptr_newLimitC);
    CuAssertIntEquals(tc, height, 0);
    height = getHeight(ptr_newLimitD);
    CuAssertIntEquals(tc, height, 0);
    
    printf("passed!\n");
}

void
TestGetBalanceFactor(CuTest *tc){
    printf("Running TestGetBalance()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);
    
    /**
     * Assert that the balance factor is correctly calculated by substracting the leftChild's height FROM the 
     * rightChild's height.
     */
    
    int balanceFactor = 0;

    balanceFactor = getBalanceFactor(ptr_root);
    CuAssertIntEquals(tc, balanceFactor, 3);
    balanceFactor = getBalanceFactor(ptr_newLimitA);
    CuAssertIntEquals(tc, balanceFactor, -1);
    balanceFactor = getBalanceFactor(ptr_newLimitB);
    CuAssertIntEquals(tc, balanceFactor, 1);
    balanceFactor = getBalanceFactor(ptr_newLimitC);
    CuAssertIntEquals(tc, balanceFactor, 0);
    balanceFactor = getBalanceFactor(ptr_newLimitD);
    CuAssertIntEquals(tc, balanceFactor, 0);

    printf("passed!\n");
}

/**
 * Test the tree operation functions.
 */

void
TestReplaceLimitInParent(CuTest *tc){
    printf("Running TestReplaceLimitInParent()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);
    /**
     * Assert that replaceLimitInParent() removes the limit from the tree and updates all relevant references.
     */

    replaceLimitInParent(ptr_newLimitA, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitC);

    free(ptr_root);
    ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    replaceLimitInParent(ptr_newLimitA, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitC->leftChild, ptr_newLimitD);
    CuAssertPtrEquals(tc, ptr_newLimitC->rightChild, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitD->parent, ptr_newLimitC);

    printf("passed!\n");

}

void
TestRemoveLimit(CuTest *tc){
    printf("Running TestRemoveLimit()..\n");
    // Setup test BST for test.
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    /**
     * Assert that removeLimit() successfully removes a limit, no matter where this limit is present at.
     */

    /**
     * TestCase1: Removing Limits which are present as children, with no children of their own.
     */

    int statusCode = 0;


    // Remove first child
    statusCode = removeLimit(ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    CuAssertPtrEquals(tc, ptr_root->leftChild->leftChild, NULL);
    CuAssertPtrEquals(tc, ptr_root->leftChild->rightChild, ptr_newLimitC);

    // Remove second child
    statusCode = removeLimit(ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);
    CuAssertPtrEquals(tc, ptr_root->leftChild->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_root->leftChild->leftChild, NULL);

    /**
     * TestCase2: Remove a limit which has a single child and parent
     */

    // Reset the test BST
    free(ptr_root);
    ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    statusCode = removeLimit(ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);
    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitB->parent->leftChild, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_newLimitB);



    CuFail(tc, "FAILURE: Finish this test!\n");
}

void
TestRotateLL(CuTest *tc){
    printf("Running TestRotateLL()..\n");
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(50.0);
    Limit *ptr_newLimitC = createDummyLimit(40.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);

    /**
     * Assert that all references are correctly updated and the pointers are correct.
     */

    rotateLeftLeft(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, ptr_newLimitC);

    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitC->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitC->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_newLimitA->parent, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitA->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitA->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitB);

    printf("passed!\n");
}

void
TestRotateLR(CuTest *tc){
    printf("Running TestRotateLR()..\n");
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(50.0);
    Limit *ptr_newLimitC = createDummyLimit(60.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);

    /**
     * Assert that all references are correctly updated and the pointers are correct.
     */

    rotateLeftRight(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitC->rightChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_newLimitC->leftChild, ptr_newLimitC);

    CuAssertPtrEquals(tc, ptr_newLimitA->parent, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitA->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitA->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitC);

    printf("passed!\n");
}

void
TestRotateRR(CuTest *tc){
    printf("Running TestRotateRR()..\n");
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(300.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);

    rotateLeftLeft(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, ptr_newLimitC);

    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitC->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitC->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_newLimitA->parent, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitA->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitA->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitB);

    printf("passed!\n");
}

void
TestRotateRL(CuTest *tc){
    printf("Running TestRotateRL()..\n");
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(150.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, statusCode, 1);

    /**
     * Assert that all references are correctly updated and the pointers are correct.
     */

    rotateLeftRight(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_newLimitC->parent, ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitC->rightChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_newLimitC->leftChild, ptr_newLimitC);

    CuAssertPtrEquals(tc, ptr_newLimitA->parent, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitA->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitA->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_newLimitB->parent, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitB->rightChild, NULL);
    CuAssertPtrEquals(tc, ptr_newLimitB->leftChild, NULL);

    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitC);

    printf("passed!\n");
}

void
TestBalanceBranch(CuTest *tc){
    printf("Running TestBalanceBranch()..\n");
    CuFail(tc, "FAILURE: Finish this test!\n");
}


/**
 * Create Test Suite and test runner.
 */

CuSuite* HFTLobGetSuite(void){
    /**
     * Prepare the test suite.
     */
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestOrderPushing);
    SUITE_ADD_TEST(suite, TestOrderPopping);
    SUITE_ADD_TEST(suite, TestCreateRoot);
    SUITE_ADD_TEST(suite, TestAddNewLimit);
    SUITE_ADD_TEST(suite, TestLimitExists);
    SUITE_ADD_TEST(suite, TestLimitIsRoot);
    SUITE_ADD_TEST(suite, TestHasGrandpa);
    SUITE_ADD_TEST(suite, TestGetGrandpa);
    SUITE_ADD_TEST(suite, TestGetMaximumLimit);
    SUITE_ADD_TEST(suite, TestGetMinimumLimit);
    SUITE_ADD_TEST(suite, TestGetHeight);
    SUITE_ADD_TEST(suite, TestGetBalanceFactor);
    SUITE_ADD_TEST(suite, TestReplaceLimitInParent);
    SUITE_ADD_TEST(suite, TestRemoveLimit);
    SUITE_ADD_TEST(suite, TestRotateLL);
    SUITE_ADD_TEST(suite, TestRotateLR);
    SUITE_ADD_TEST(suite, TestRotateRR);
    SUITE_ADD_TEST(suite, TestRotateRL);

    return suite;
}

CuSuite* HFTLobGetSuite();

void RunAllTests(void){
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, HFTLobGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}
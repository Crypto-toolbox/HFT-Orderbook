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

void
TestCreateDummyTree(CuTest *tc){
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(200.0);
    Limit *ptr_newLimitC = createDummyLimit(50.0);
    Limit *ptr_newLimitD = createDummyLimit(45.0);
    Limit *ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_root->rightChild);

    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitA->parent);
    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitA->leftChild);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitA->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->leftChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitC->parent);
    CuAssertPtrEquals(tc, ptr_newLimitD, ptr_newLimitC->leftChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitC->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitD->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitD->leftChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitD->rightChild);
}

/**
 * Test Functions related to adding and removing orders to/from a limit.
 */

void
TestOrderPushing(CuTest *tc){
    Limit *ptr_limit = createDummyLimit(1000.0);
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
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->tailOrder->parentLimit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 10000.0, ptr_limit->totalVolume, 0.0);
    CuAssertDblEquals(tc, 10.0, ptr_limit->size, 0.0);
    CuAssertIntEquals(tc, 1, ptr_limit->orderCount);
    expected_size = ptr_limit->size;
    expected_volume = ptr_limit->totalVolume;
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
    CuAssertIntEquals(tc, 1, returnCode);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->tailOrder->parentLimit);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->headOrder->parentLimit);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->headOrder->nextOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->headOrder->prevOrder);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->tailOrder->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->tailOrder->nextOrder);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 30000.0, ptr_limit->totalVolume, 0.0);
    CuAssertDblEquals(tc, 30.0, ptr_limit->size, 0.0);
    CuAssertIntEquals(tc, 2, ptr_limit->orderCount);

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
    CuAssertIntEquals(tc, 1, returnCode);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->headOrder->parentLimit);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->tailOrder->parentLimit);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->headOrder->parentLimit);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->headOrder->nextOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->headOrder->prevOrder);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->tailOrder->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->tailOrder->nextOrder);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->tailOrder->prevOrder->nextOrder);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->tailOrder->prevOrder->prevOrder);
    CuAssertPtrEquals(tc, ptr_limit, ptr_limit->tailOrder->prevOrder->parentLimit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 60000.0, ptr_limit->totalVolume, 0.0);
    CuAssertDblEquals(tc, 60.0, ptr_limit->size, 0.0);
    CuAssertIntEquals(tc, 3, ptr_limit->orderCount);
    expected_size = ptr_limit->size;
    expected_volume = ptr_limit->totalVolume;
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
    CuAssertIntEquals(tc, 0, returnCode);
}

void
TestOrderPopping(CuTest *tc){
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

    Order *ptr_poppedOrder = malloc(sizeof(Order));

    /**
     * Pop all orders and check after each pop that all references are updated correctly, along with all relevant
     * attributes in the limit.
     */

    // // // //
    // Pop # 1
    // // // //
    ptr_poppedOrder = popOrder(ptr_limit);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_poppedOrder);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->tailOrder->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->tailOrder->nextOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->headOrder->prevOrder);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_limit->headOrder->nextOrder);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 50000.0, limit.totalVolume, 0.0);
    CuAssertDblEquals(tc, 50.0, limit.size, 0.0);
    CuAssertIntEquals(tc, 2, limit.orderCount);


    // // // //
    // Pop # 2
    // // // //
    ptr_poppedOrder = popOrder(ptr_limit);
    CuAssertPtrEquals(tc, ptr_newOrderB, ptr_poppedOrder);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderC->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderC->nextOrder);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 30000.0, limit.totalVolume, 0.0);
    CuAssertDblEquals(tc, 30.0, limit.size, 0.0);
    CuAssertIntEquals(tc, 1, limit.orderCount);

    // // // //
    // Pop # 3
    // // // //
    ptr_poppedOrder = popOrder(ptr_limit);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_poppedOrder);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, NULL, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->tailOrder);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, 0.0, limit.totalVolume, 0.0);
    CuAssertDblEquals(tc, 0.0, limit.size, 0.0);
    CuAssertIntEquals(tc, 0, limit.orderCount);

    // // // //
    // Pop # 4
    // // // //
    ptr_poppedOrder = popOrder(ptr_limit);
    CuAssertPtrEquals(tc, NULL, ptr_poppedOrder);

    free(ptr_poppedOrder);
}

void
TestRemoveOrder(CuTest *tc){
    Limit *ptr_limit = createDummyLimit(1000.0);
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

    int returnCode = 0;

    returnCode = pushOrder(ptr_limit, ptr_newOrderA);
    CuAssertIntEquals(tc, returnCode, 1);

    returnCode = pushOrder(ptr_limit, ptr_newOrderB);
    CuAssertIntEquals(tc, 1, returnCode);

    returnCode = pushOrder(ptr_limit, ptr_newOrderC);
    CuAssertIntEquals(tc, 1, returnCode);

    /**
     * Assert that removing an order from the middle works correctly and all references are updated
     * correctly.
     */
    returnCode = removeOrder(ptr_newOrderB);
    CuAssertIntEquals(tc, 1, returnCode);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_newOrderC->nextOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderC->prevOrder);
    CuAssertPtrEquals(tc, ptr_newOrderA, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_newOrderA->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderA->nextOrder);

    returnCode = removeOrder(ptr_newOrderA);
    CuAssertIntEquals(tc, 1, returnCode);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, ptr_newOrderC, ptr_limit->tailOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderC->prevOrder);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderC->nextOrder);

    returnCode = removeOrder(ptr_newOrderC);
    CuAssertIntEquals(tc, 1, returnCode);
    CuAssertPtrEquals(tc, NULL, ptr_limit->headOrder);
    CuAssertPtrEquals(tc, NULL, ptr_limit->tailOrder);
}

/**
 * Test Root creation and tree population funcs.
 */

void
TestCreateRoot(CuTest *tc){
    /**
     * Test the createRoot() function and assert it creates a root with limitPrice of -INFINITY, no children
     * and no parent.
     */
    Limit *ptr_root = createRoot();
    CuAssertPtrEquals(tc, NULL, ptr_root->parent);
    CuAssertTrue(tc, isinf(ptr_root->limitPrice));
    CuAssertPtrEquals(tc, NULL, ptr_root->leftChild);
    CuAssertPtrEquals(tc, NULL, ptr_root->rightChild);
}

void
TestAddNewLimit(CuTest *tc){
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
    CuAssertIntEquals(tc, 1, statusCode);

    CuAssertPtrEquals(tc, ptr_root->rightChild, ptr_newLimitA);
    CuAssertPtrEquals(tc, ptr_root->rightChild->parent, ptr_root);

    /**
      * Add the second limit. Assert it is added as root->leftChild->leftChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, 1, statusCode);

    CuAssertPtrEquals(tc, ptr_root->rightChild->leftChild, ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_root->rightChild->leftChild->parent, ptr_newLimitA);

    /**
      * Add the third limit. Assert it is added as root->leftChild->rightChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, 1, statusCode);

    CuAssertPtrEquals(tc, ptr_root->rightChild->rightChild, ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_root->rightChild->rightChild->parent, ptr_newLimitA);
    /**
     * Add a duplicate limit and assert the returned status code is 0.
     */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, 0, statusCode);
}

/**
 * Test the convenience functions for their correctness.
 */

void
TestLimitExists(CuTest *tc){
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
    CuAssertIntEquals(tc, 1, statusCode);
    limit.limitPrice = 100;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, 1, statusCode);
    limit.limitPrice = 200;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, 1, statusCode);
    limit.limitPrice = 50;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, 1, statusCode);
    limit.limitPrice = 500;
    statusCode = limitExists(ptr_root, ptr_limit);
    CuAssertIntEquals(tc, 0, statusCode);
}

void
TestLimitIsRoot(CuTest *tc){
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
    CuAssertIntEquals(tc, 0, statusCode);
    statusCode = limitIsRoot(ptr_newLimitB);
    CuAssertIntEquals(tc, 0, statusCode);
    statusCode = limitIsRoot(ptr_newLimitC);
    CuAssertIntEquals(tc, 0, statusCode);
    statusCode = limitIsRoot(ptr_root);
    CuAssertIntEquals(tc, 1, statusCode);
}

void
TestHasGrandpa(CuTest *tc){
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
    CuAssertIntEquals(tc, 0, statusCode);
    statusCode = hasGrandpa(ptr_newLimitB);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = hasGrandpa(ptr_newLimitC);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = hasGrandpa(ptr_root);
    CuAssertIntEquals(tc, 0, statusCode);
}

void
TestGetGrandpa(CuTest *tc){
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
    CuAssertPtrEquals(tc, NULL, ptr_grandpa);
    ptr_grandpa = getGrandpa(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_root, ptr_grandpa);
    ptr_grandpa = getGrandpa(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_root, ptr_grandpa);
    ptr_grandpa = getGrandpa(ptr_root);
    CuAssertPtrEquals(tc, NULL, ptr_grandpa);
}

void
TestGetMaximumLimit(CuTest *tc){
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
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_retValue);
    ptr_retValue = getMaximumLimit(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_retValue);
    ptr_retValue = getMaximumLimit(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_retValue);
    ptr_retValue = getMaximumLimit(ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_retValue);
}

void
TestGetMinimumLimit(CuTest *tc){
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
    CuAssertPtrEquals(tc, ptr_newLimitD, ptr_retValue);
    ptr_retValue = getMinimumLimit(ptr_newLimitB);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_retValue);
    ptr_retValue = getMinimumLimit(ptr_newLimitC);
    CuAssertPtrEquals(tc, ptr_newLimitD, ptr_retValue);
    ptr_retValue = getMinimumLimit(ptr_root);
    CuAssertPtrEquals(tc, ptr_newLimitD, ptr_retValue);
}

void
TestGetHeight(CuTest *tc){
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
    CuAssertIntEquals(tc, 3, height);
    height = getHeight(ptr_newLimitA);
    CuAssertIntEquals(tc, 2, height);
    height = getHeight(ptr_newLimitB);
    CuAssertIntEquals(tc, 0, height);
    height = getHeight(ptr_newLimitC);
    CuAssertIntEquals(tc, 1, height);
    height = getHeight(ptr_newLimitD);
    CuAssertIntEquals(tc, 0, height);
}

void
TestGetBalanceFactor(CuTest *tc){
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

    balanceFactor = getBalanceFactor(ptr_newLimitA);
    CuAssertIntEquals(tc, -1, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitB);
    CuAssertIntEquals(tc, 0, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitC);
    CuAssertIntEquals(tc, -1, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitD);
    CuAssertIntEquals(tc, 0, balanceFactor);

    free(ptr_root);
    ptr_newLimitA->limitPrice = 100.0;
    ptr_newLimitB->limitPrice =200.0;
    ptr_newLimitC->limitPrice = 250.0;
    ptr_newLimitD->limitPrice = 45.0;
    ptr_root = createDummyTree(ptr_newLimitA, ptr_newLimitB, ptr_newLimitC, ptr_newLimitD);

    balanceFactor = getBalanceFactor(ptr_newLimitA);
    CuAssertIntEquals(tc, 1, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitB);
    CuAssertIntEquals(tc, 1, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitC);
    CuAssertIntEquals(tc, 0, balanceFactor);
    balanceFactor = getBalanceFactor(ptr_newLimitD);
    CuAssertIntEquals(tc, 0, balanceFactor);
}

/**
 * Test the tree operation functions.
 */

void
TestReplaceLimitInParent(CuTest *tc){
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
    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_root->rightChild);
}

void
TestRemoveLimit(CuTest *tc){
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

    statusCode = removeLimit(ptr_newLimitB);
    CuAssertIntEquals(tc, 1, statusCode);
    CuAssertPtrEquals(tc, NULL,  ptr_root->rightChild->rightChild);
    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_root->rightChild->leftChild);


    /**
     * TestCase2: Remove a limit which has a single child and parent
     */

    statusCode = removeLimit(ptr_newLimitC);
    CuAssertIntEquals(tc, 1, statusCode);
    CuAssertPtrEquals(tc, NULL, ptr_root->rightChild->rightChild);
    CuAssertPtrEquals(tc, ptr_newLimitD, ptr_root->rightChild->leftChild);

    /**
     * TestCase3: Remove a limit with two children and a parent.
     */

    /* Reset the BST. */
    free(ptr_root);
    Limit *ptr_LimitA = createDummyLimit(100.0);
    Limit *ptr_LimitB = createDummyLimit(200.0);
    Limit *ptr_LimitC = createDummyLimit(50.0);
    Limit *ptr_LimitD = createDummyLimit(45.0);
    Limit *ptr_rootB = createDummyTree(ptr_LimitA, ptr_LimitB, ptr_LimitC, ptr_LimitD);

    statusCode = removeLimit(ptr_LimitA);
    CuAssertIntEquals(tc, 1, statusCode);
    CuAssertPtrEquals(tc, ptr_LimitB, ptr_rootB->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_rootB->rightChild->rightChild);
    CuAssertPtrEquals(tc, ptr_LimitC, ptr_rootB->rightChild->leftChild);
    CuAssertPtrEquals(tc, ptr_LimitD, ptr_rootB->rightChild->leftChild->leftChild);
}

void
TestRotateLL(CuTest *tc){
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(50.0);
    Limit *ptr_newLimitC = createDummyLimit(40.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, 1, statusCode);

    /**
     * Assert that all references are correctly updated and the pointers are correct.
     */

    rotateLeftLeft(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitB->rightChild);
    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitB->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitC->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitC->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitC->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitA->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_root->rightChild);
}

void
TestRotateLR(CuTest *tc){
    // Setup the BST Tree
    Limit *ptr_newLimitA = createDummyLimit(100.0);
    Limit *ptr_newLimitB = createDummyLimit(50.0);
    Limit *ptr_newLimitC = createDummyLimit(60.0);
    Limit *ptr_root = createRoot();

    int statusCode = 0;

    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuAssertIntEquals(tc, 1, statusCode);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuAssertIntEquals(tc, 1, statusCode);

    /**
     * Assert that all references are correctly updated and the pointers are correct.
     */

    rotateLeftRight(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitC->parent);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitC->leftChild);
    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitC->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitA->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_root->rightChild);
}

void
TestRotateRR(CuTest *tc){
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

    rotateRightRight(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitB->leftChild);
    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitB->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitC->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitC->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitC->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitA->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_root->rightChild);
}

void
TestRotateRL(CuTest *tc){
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

    rotateRightLeft(ptr_newLimitA);

    CuAssertPtrEquals(tc, ptr_root, ptr_newLimitC->parent);
    CuAssertPtrEquals(tc, ptr_newLimitA, ptr_newLimitC->leftChild);
    CuAssertPtrEquals(tc, ptr_newLimitB, ptr_newLimitC->rightChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitA->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitA->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_newLimitB->parent);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->rightChild);
    CuAssertPtrEquals(tc, NULL, ptr_newLimitB->leftChild);

    CuAssertPtrEquals(tc, ptr_newLimitC, ptr_root->rightChild);
}

void
TestBalanceBranch(CuTest *tc){
    CuFail(tc, "Finish this test!\n");
}


/**
 * Create Test Suite and test runner.
 */

CuSuite* HFTLobGetSuite(void){
    /**
     * Prepare the test suite.
     */
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestCreateDummyTree);
    SUITE_ADD_TEST(suite, TestOrderPushing);
    SUITE_ADD_TEST(suite, TestOrderPopping);
    SUITE_ADD_TEST(suite, TestRemoveOrder);
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
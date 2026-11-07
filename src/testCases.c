#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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
    Limit *ptr_root = createRoot();
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
    newOrderA.id = 1234;

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
    newOrderB.id = 1235;

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
    newOrderC.id = 1236;

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
    newOrderC.id = 1236;
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
    newOrderA.id = 1234;

    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    initOrder(ptr_newOrderB);
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.id = 1235;

    Order newOrderC;
    Order *ptr_newOrderC = &newOrderC;
    initOrder(ptr_newOrderC);
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.id = 1236;

    pushOrder(ptr_limit, ptr_newOrderA);
    pushOrder(ptr_limit, ptr_newOrderB);
    pushOrder(ptr_limit, ptr_newOrderC);

    Order *ptr_poppedOrder = NULL;

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
    newOrderA.id = 1234;

    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    initOrder(ptr_newOrderB);
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.id = 1235;

    Order newOrderC;
    Order *ptr_newOrderC = &newOrderC;
    initOrder(ptr_newOrderC);
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.id = 1236;

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
    CuAssertIntEquals(tc, 2, ptr_limit->orderCount);
    CuAssertDblEquals(tc, 40.0, ptr_limit->size, 0.0);
    CuAssertDblEquals(tc, 40000.0, ptr_limit->totalVolume, 0.0);
    CuAssertPtrEquals(tc, NULL, ptr_newOrderB->parentLimit);
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
    CuAssertIntEquals(tc, 0, ptr_limit->orderCount);
    CuAssertDblEquals(tc, 0.0, ptr_limit->size, 0.0);

    /* Removing an order that is no longer in a limit fails gracefully. */
    CuAssertIntEquals(tc, -1, removeOrder(ptr_newOrderC));
    free(ptr_limit);
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
TestQueueIsFifo(CuTest *tc){
    Limit *a = createDummyLimit(1.0);
    Limit *b = createDummyLimit(2.0);
    Limit *c = createDummyLimit(3.0);
    Queue q;
    initQueue(&q);

    CuAssertIntEquals(tc, 1, queueIsEmpty(&q));
    pushToQueue(&q, a);
    pushToQueue(&q, b);
    CuAssertPtrEquals(tc, a, popFromQueue(&q));
    pushToQueue(&q, c);
    CuAssertIntEquals(tc, 2, q.size);
    CuAssertPtrEquals(tc, b, popFromQueue(&q));
    CuAssertPtrEquals(tc, c, popFromQueue(&q));
    CuAssertPtrEquals(tc, NULL, popFromQueue(&q));
    CuAssertIntEquals(tc, 1, queueIsEmpty(&q));
    CuAssertIntEquals(tc, 0, q.size);
    free(a); free(b); free(c);
}

void
TestInitLimitClearsTail(CuTest *tc){
    Limit limit;
    limit.tailOrder = (Order *)&limit; /* garbage */
    initLimit(&limit);
    CuAssertPtrEquals(tc, NULL, limit.headOrder);
    CuAssertPtrEquals(tc, NULL, limit.tailOrder);
}

void
TestRemoveLimitWithDeepSuccessor(CuTest *tc){
    /**
     * Regression test for #34: removing a node with two children whose
     * in-order successor is NOT its direct right child.
     *
     *          100                 120
     *        /     \             /     \
     *      50      150   ->    50      150
     *             /   \               /   \
     *           120   200           130   200
     *              \
     *              130
     */
    Limit *l100 = createDummyLimit(100.0);
    Limit *l50  = createDummyLimit(50.0);
    Limit *l150 = createDummyLimit(150.0);
    Limit *l120 = createDummyLimit(120.0);
    Limit *l200 = createDummyLimit(200.0);
    Limit *l130 = createDummyLimit(130.0);
    Limit *root = createRoot();
    addNewLimit(root, l100);
    addNewLimit(root, l50);
    addNewLimit(root, l150);
    addNewLimit(root, l120);
    addNewLimit(root, l200);
    addNewLimit(root, l130);

    CuAssertIntEquals(tc, 1, removeLimit(l100));
    CuAssertPtrEquals(tc, l120, root->rightChild);
    CuAssertPtrEquals(tc, root, l120->parent);
    CuAssertPtrEquals(tc, l50, l120->leftChild);
    CuAssertPtrEquals(tc, l120, l50->parent);
    CuAssertPtrEquals(tc, l150, l120->rightChild);
    CuAssertPtrEquals(tc, l120, l150->parent);
    CuAssertPtrEquals(tc, l130, l150->leftChild);
    CuAssertPtrEquals(tc, l150, l130->parent);
    CuAssertPtrEquals(tc, l200, l150->rightChild);
    CuAssertIntEquals(tc, 0, limitExists(root, l100));
    CuAssertIntEquals(tc, 2, getHeight(l120));

    /* Remove a node whose successor IS its direct right child. */
    CuAssertIntEquals(tc, 1, removeLimit(l150));
    CuAssertPtrEquals(tc, l200, l120->rightChild);
    CuAssertPtrEquals(tc, l120, l200->parent);
    CuAssertPtrEquals(tc, l130, l200->leftChild);
    CuAssertPtrEquals(tc, l200, l130->parent);
    CuAssertPtrEquals(tc, NULL, l200->rightChild);
}

void
TestBalanceBranch(CuTest *tc){
    /* A right-leaning chain 10 -> 20 -> 30 is rebalanced around 20. */
    Limit *l10 = createDummyLimit(10.0);
    Limit *l20 = createDummyLimit(20.0);
    Limit *l30 = createDummyLimit(30.0);
    Limit *root = createRoot();
    addNewLimit(root, l10);
    addNewLimit(root, l20);
    addNewLimit(root, l30);

    balanceBranch(l10);
    CuAssertPtrEquals(tc, l20, root->rightChild);
    CuAssertPtrEquals(tc, l10, l20->leftChild);
    CuAssertPtrEquals(tc, l30, l20->rightChild);
    CuAssertIntEquals(tc, 0, getBalanceFactor(l20));

    /* Calling it on an already balanced branch is a no-op. */
    balanceBranch(l20);
    CuAssertPtrEquals(tc, l20, root->rightChild);
}




/**
 * Hash map and pool tests.
 */

void
TestMapBasics(CuTest *tc){
    HashMap map;
    int a = 1, b = 2;
    CuAssertIntEquals(tc, 1, initMap(&map, 0));
    CuAssertPtrEquals(tc, NULL, mapGet(&map, 42));
    CuAssertIntEquals(tc, 1, mapPut(&map, 42, &a));
    CuAssertIntEquals(tc, 1, mapPut(&map, 0, &b));          /* key 0 is valid */
    CuAssertIntEquals(tc, 1, mapPut(&map, UINT64_MAX, &b));
    CuAssertPtrEquals(tc, &a, mapGet(&map, 42));
    CuAssertPtrEquals(tc, &b, mapGet(&map, 0));
    CuAssertIntEquals(tc, 1, mapPut(&map, 42, &b));         /* overwrite */
    CuAssertPtrEquals(tc, &b, mapGet(&map, 42));
    CuAssertIntEquals(tc, 3, (int)map.size);
    CuAssertIntEquals(tc, 0, mapPut(&map, 7, NULL));        /* NULL values refused */
    CuAssertPtrEquals(tc, &b, mapRemove(&map, 42));
    CuAssertPtrEquals(tc, NULL, mapRemove(&map, 42));
    CuAssertPtrEquals(tc, NULL, mapGet(&map, 42));
    CuAssertIntEquals(tc, 2, (int)map.size);
    freeMap(&map);
}

void
TestMapRandomAgainstReference(CuTest *tc){
    /**
     * Random puts/removes over a small key space (lots of probe-run
     * collisions and backward shifts), checked against a plain array.
     */
    enum { KEYS = 2000, OPS = 30000 };
    static int values[KEYS];
    static int present[KEYS];
    HashMap map;
    int op, k, count = 0;
    initMap(&map, 0);
    srand(7);
    for(op = 0; op < OPS; ++op){
        k = rand() % KEYS;
        if(rand() % 3 == 0){
            void *removed = mapRemove(&map, (uint64_t)k * 1000003u);
            CuAssertPtrEquals(tc, present[k] ? &values[k] : NULL, removed);
            count -= present[k];
            present[k] = 0;
        }
        else{
            CuAssertIntEquals(tc, 1, mapPut(&map, (uint64_t)k * 1000003u, &values[k]));
            count += !present[k];
            present[k] = 1;
        }
        CuAssertIntEquals(tc, count, (int)map.size);
        CuAssertTrue(tc, map.size * 2 <= map.capacity);
    }
    for(k = 0; k < KEYS; ++k){
        CuAssertPtrEquals(tc, present[k] ? &values[k] : NULL,
                          mapGet(&map, (uint64_t)k * 1000003u));
    }
    freeMap(&map);
}

void
TestPoolReuseAndGrowth(CuTest *tc){
    enum { N = 1000 };
    static Order *objs[N];
    Pool pool;
    int i, j;
    initPool(&pool, sizeof(Order));

    /* Many allocations across several slabs: all distinct and aligned. */
    for(i = 0; i < N; ++i){
        objs[i] = poolAlloc(&pool);
        CuAssertPtrNotNull(tc, objs[i]);
        CuAssertTrue(tc, ((uintptr_t)objs[i] % 16) == 0);
        initOrder(objs[i]);
        objs[i]->id = (OrderId)i;
    }
    for(i = 0; i < N; i += 97){
        for(j = i + 1; j < N; j += 89){
            CuAssertTrue(tc, objs[i] != objs[j]);
        }
    }
    for(i = 0; i < N; ++i){
        CuAssertTrue(tc, objs[i]->id == (OrderId)i);
    }

    /* LIFO reuse: the most recently freed object comes back first. */
    poolFree(&pool, objs[10]);
    poolFree(&pool, objs[20]);
    CuAssertPtrEquals(tc, objs[20], poolAlloc(&pool));
    CuAssertPtrEquals(tc, objs[10], poolAlloc(&pool));
    poolFree(&pool, NULL);
    destroyPool(&pool);
    CuAssertPtrEquals(tc, NULL, pool.slabs);
}

/**
 * Book API tests.
 */

void
TestBookAddAndBestPrices(CuTest *tc){
    Book *book = createBook();
    CuAssertPtrNotNull(tc, book);
    CuAssertPtrEquals(tc, NULL, getBestBid(book));
    CuAssertPtrEquals(tc, NULL, getBestAsk(book));

    Order *a = addOrderToBook(book, 1, HFTLOB_BUY, 5, 100.0);
    Order *b = addOrderToBook(book, 2, HFTLOB_BUY, 3, 101.0);
    Order *c = addOrderToBook(book, 3, HFTLOB_BUY, 2, 101.0);
    Order *d = addOrderToBook(book, 4, HFTLOB_SELL, 4, 103.0);
    Order *e = addOrderToBook(book, 5, HFTLOB_SELL, 1, 102.0);
    CuAssertPtrNotNull(tc, a);
    CuAssertPtrNotNull(tc, b);
    CuAssertPtrNotNull(tc, c);
    CuAssertPtrNotNull(tc, d);
    CuAssertPtrNotNull(tc, e);

    CuAssertDblEquals(tc, 101.0, getBestBid(book)->limitPrice, 0.0);
    CuAssertDblEquals(tc, 102.0, getBestAsk(book)->limitPrice, 0.0);

    /* Volume at limit, and FIFO: b is older than c, so b sits at the tail. */
    Limit *l101 = getLimitFromBook(book, HFTLOB_BUY, 101.0);
    CuAssertIntEquals(tc, 2, l101->orderCount);
    CuAssertDblEquals(tc, 5.0, l101->size, 0.0);
    CuAssertDblEquals(tc, 505.0, l101->totalVolume, 0.0);
    CuAssertPtrEquals(tc, b, l101->tailOrder);
    CuAssertPtrEquals(tc, c, l101->headOrder);

    /* Lookups by id; ids use the full 64-bit range. */
    OrderId bigId = UINT64_MAX;
    Order *z = addOrderToBook(book, bigId, HFTLOB_SELL, 1, 110.0);
    CuAssertPtrEquals(tc, z, getOrderFromBook(book, bigId));
    CuAssertPtrEquals(tc, NULL, getOrderFromBook(book, 25));
    CuAssertTrue(tc, z->id == bigId);
    CuAssertPtrEquals(tc, NULL, getLimitFromBook(book, HFTLOB_SELL, 101.0));

    freeBook(book);
}

void
TestBookRejectsInvalidOrders(CuTest *tc){
    Book *book = createBook();
    CuAssertPtrNotNull(tc, addOrderToBook(book, 1, HFTLOB_BUY, 1, 100.0));
    CuAssertPtrEquals(tc, NULL, addOrderToBook(book, 1, HFTLOB_SELL, 1, 105.0));
    CuAssertPtrEquals(tc, NULL, addOrderToBook(book, 2, 7, 1, 100.0));
    CuAssertPtrEquals(tc, NULL, addOrderToBook(book, 2, HFTLOB_BUY, 0, 100.0));
    CuAssertPtrEquals(tc, NULL, addOrderToBook(book, 2, HFTLOB_BUY, 1, NAN));
    CuAssertIntEquals(tc, 1, (int)book->orders.size);
    CuAssertIntEquals(tc, 0, removeOrderFromBook(book, 999));
    CuAssertIntEquals(tc, 0, replaceOrderInBook(book, 999, 1, 1));
    CuAssertIntEquals(tc, -1, replaceOrderInBook(book, 1, 1, NAN));
    freeBook(book);
}

void
TestBookRemove(CuTest *tc){
    Book *book = createBook();
    addOrderToBook(book, 1, HFTLOB_BUY, 5, 100.0);
    addOrderToBook(book, 2, HFTLOB_BUY, 3, 101.0);
    addOrderToBook(book, 3, HFTLOB_BUY, 2, 101.0);

    /* Removing one of two orders keeps the level. */
    CuAssertIntEquals(tc, 1, removeOrderFromBook(book, 2));
    CuAssertPtrEquals(tc, NULL, getOrderFromBook(book, 2));
    CuAssertDblEquals(tc, 101.0, getBestBid(book)->limitPrice, 0.0);
    CuAssertDblEquals(tc, 2.0, getBestBid(book)->size, 0.0);

    /* Emptying the best level falls back to the next best price. */
    CuAssertIntEquals(tc, 1, removeOrderFromBook(book, 3));
    CuAssertPtrEquals(tc, NULL, getLimitFromBook(book, HFTLOB_BUY, 101.0));
    CuAssertDblEquals(tc, 100.0, getBestBid(book)->limitPrice, 0.0);

    CuAssertIntEquals(tc, 1, removeOrderFromBook(book, 1));
    CuAssertPtrEquals(tc, NULL, getBestBid(book));
    CuAssertPtrEquals(tc, NULL, book->buyTree->rightChild);
    CuAssertIntEquals(tc, 0, (int)book->orders.size);
    freeBook(book);
}

void
TestBookReplace(CuTest *tc){
    Book *book = createBook();
    Order *a = addOrderToBook(book, 1, HFTLOB_SELL, 5, 100.0);
    Order *b = addOrderToBook(book, 2, HFTLOB_SELL, 3, 100.0);

    /* Size change: in place, queue position kept. */
    CuAssertIntEquals(tc, 1, replaceOrderInBook(book, 1, 2, 100.0));
    Limit *l100 = getLimitFromBook(book, HFTLOB_SELL, 100.0);
    CuAssertDblEquals(tc, 2.0, a->shares, 0.0);
    CuAssertDblEquals(tc, 5.0, l100->size, 0.0);
    CuAssertDblEquals(tc, 500.0, l100->totalVolume, 0.0);
    CuAssertPtrEquals(tc, a, l100->tailOrder);

    /* Price change: same Order moves to the back of its new level. */
    addOrderToBook(book, 3, HFTLOB_SELL, 1, 99.0);
    CuAssertIntEquals(tc, 1, replaceOrderInBook(book, 1, 2, 99.0));
    CuAssertPtrEquals(tc, a, getOrderFromBook(book, 1));
    CuAssertDblEquals(tc, 99.0, a->limit, 0.0);
    Limit *l99 = getLimitFromBook(book, HFTLOB_SELL, 99.0);
    CuAssertPtrEquals(tc, l99, a->parentLimit);
    CuAssertPtrEquals(tc, a, l99->headOrder);
    CuAssertDblEquals(tc, 3.0, l99->size, 0.0);
    CuAssertDblEquals(tc, 3.0, l100->size, 0.0);
    CuAssertPtrEquals(tc, b, l100->tailOrder);

    /* Moving the last order off a level removes the level. */
    CuAssertIntEquals(tc, 1, replaceOrderInBook(book, 2, 3, 98.0));
    CuAssertPtrEquals(tc, NULL, getLimitFromBook(book, HFTLOB_SELL, 100.0));
    CuAssertDblEquals(tc, 98.0, getBestAsk(book)->limitPrice, 0.0);

    /* Zero size cancels. */
    CuAssertIntEquals(tc, 1, replaceOrderInBook(book, 2, 0, 98.0));
    CuAssertPtrEquals(tc, NULL, getOrderFromBook(book, 2));
    CuAssertDblEquals(tc, 99.0, getBestAsk(book)->limitPrice, 0.0);
    freeBook(book);
}

void
TestBookRotationsKeepParentLinks(CuTest *tc){
    /* Ascending prices force repeated rotations involving subtrees. */
    Book *book = createBook();
    int i;
    for(i = 0; i < 64; ++i){
        addOrderToBook(book, (OrderId)i, HFTLOB_BUY, 1, (double)i);
    }
    Limit *top = book->buyTree->rightChild;
    CuAssertIntEquals(tc, 6, top->height);
    CuAssertIntEquals(tc, 6, getHeight(top));
    CuAssertDblEquals(tc, 63.0, getBestBid(book)->limitPrice, 0.0);
    freeBook(book);
}


typedef struct FillLog{
    int count;
    OrderId ids[16];
    double shares[16];
    double prices[16];
} FillLog;

static void
recordFill(const Order *resting, double shares, double price, void *ctx){
    FillLog *log = ctx;
    if(log->count < 16){
        log->ids[log->count] = resting->id;
        log->shares[log->count] = shares;
        log->prices[log->count] = price;
    }
    log->count++;
}

void
TestBookExecuteBuy(CuTest *tc){
    Book *book = createBook();
    FillLog log = {0};
    addOrderToBook(book, 1, HFTLOB_SELL, 5, 101.0);
    addOrderToBook(book, 2, HFTLOB_SELL, 3, 101.0);   /* behind 1 in the queue */
    addOrderToBook(book, 3, HFTLOB_SELL, 4, 102.0);
    addOrderToBook(book, 4, HFTLOB_SELL, 9, 105.0);
    addOrderToBook(book, 5, HFTLOB_BUY, 9, 100.0);    /* own side: untouched */

    /* Buy 10 up to 102: fills 1 (5) and 2 (3) at 101, then 2 of 3 at 102. */
    double filled = executeInBook(book, HFTLOB_BUY, 10, 102.0, recordFill, &log);
    CuAssertDblEquals(tc, 10.0, filled, 0.0);
    CuAssertIntEquals(tc, 3, log.count);
    CuAssertTrue(tc, log.ids[0] == 1 && log.ids[1] == 2 && log.ids[2] == 3);
    CuAssertDblEquals(tc, 5.0, log.shares[0], 0.0);
    CuAssertDblEquals(tc, 3.0, log.shares[1], 0.0);
    CuAssertDblEquals(tc, 2.0, log.shares[2], 0.0);
    CuAssertDblEquals(tc, 101.0, log.prices[0], 0.0);
    CuAssertDblEquals(tc, 102.0, log.prices[2], 0.0);

    /* Fully filled orders and the emptied level are gone; 3 is reduced. */
    CuAssertPtrEquals(tc, NULL, getOrderFromBook(book, 1));
    CuAssertPtrEquals(tc, NULL, getOrderFromBook(book, 2));
    CuAssertPtrEquals(tc, NULL, getLimitFromBook(book, HFTLOB_SELL, 101.0));
    CuAssertDblEquals(tc, 2.0, getOrderFromBook(book, 3)->shares, 0.0);
    CuAssertDblEquals(tc, 102.0, getBestAsk(book)->limitPrice, 0.0);
    CuAssertDblEquals(tc, 2.0, getBestAsk(book)->size, 0.0);
    CuAssertDblEquals(tc, 204.0, getBestAsk(book)->totalVolume, 0.0);
    CuAssertDblEquals(tc, 9.0, getBestBid(book)->size, 0.0);

    /* Limit price stops matching: best ask 102 is above 101.5. */
    log.count = 0;
    filled = executeInBook(book, HFTLOB_BUY, 50, 101.5, recordFill, &log);
    CuAssertDblEquals(tc, 0.0, filled, 0.0);
    CuAssertIntEquals(tc, 0, log.count);

    /* Market buy larger than the book: takes everything, returns the fill. */
    filled = executeInBook(book, HFTLOB_BUY, 50, INFINITY, NULL, NULL);
    CuAssertDblEquals(tc, 11.0, filled, 0.0);
    CuAssertPtrEquals(tc, NULL, getBestAsk(book));
    CuAssertIntEquals(tc, 1, (int)book->orders.size);
    freeBook(book);
}

void
TestBookExecuteSell(CuTest *tc){
    Book *book = createBook();
    addOrderToBook(book, 1, HFTLOB_BUY, 5, 99.0);
    addOrderToBook(book, 2, HFTLOB_BUY, 5, 100.0);
    addOrderToBook(book, 3, HFTLOB_BUY, 5, 98.0);

    /* Sell 7 down to 99: 5 at 100, then 2 at 99. 98 is out of range. */
    CuAssertDblEquals(tc, 7.0, executeInBook(book, HFTLOB_SELL, 7, 99.0, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 99.0, getBestBid(book)->limitPrice, 0.0);
    CuAssertDblEquals(tc, 3.0, getBestBid(book)->size, 0.0);
    CuAssertDblEquals(tc, 3.0, executeInBook(book, HFTLOB_SELL, 10, 99.0, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 98.0, getBestBid(book)->limitPrice, 0.0);

    /* Market sell. */
    CuAssertDblEquals(tc, 5.0, executeInBook(book, HFTLOB_SELL, 10, -INFINITY, NULL, NULL), 0.0);
    CuAssertPtrEquals(tc, NULL, getBestBid(book));
    freeBook(book);
}

void
TestBookExecuteRejectsInvalidInput(CuTest *tc){
    Book *book = createBook();
    addOrderToBook(book, 1, HFTLOB_SELL, 5, 101.0);
    CuAssertDblEquals(tc, 0.0, executeInBook(book, 7, 1, INFINITY, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 0.0, executeInBook(book, HFTLOB_BUY, 0, INFINITY, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 0.0, executeInBook(book, HFTLOB_BUY, -1, INFINITY, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 0.0, executeInBook(book, HFTLOB_BUY, 1, NAN, NULL, NULL), 0.0);
    CuAssertDblEquals(tc, 5.0, getBestAsk(book)->size, 0.0);
    freeBook(book);
}

/* --- Randomised invariant test ------------------------------------- */

static int
checkSubtree(CuTest *tc, Limit *limit, Limit *parent, double lo, double hi, int *count){
    int left, right;
    double size = 0;
    int orders = 0;
    Order *order;
    if(limit == NULL){
        return -1;
    }
    CuAssertPtrEquals(tc, parent, limit->parent);
    CuAssertTrue(tc, limit->limitPrice > lo && limit->limitPrice < hi);
    left = checkSubtree(tc, limit->leftChild, limit, lo, limit->limitPrice, count);
    right = checkSubtree(tc, limit->rightChild, limit, limit->limitPrice, hi, count);
    CuAssertTrue(tc, abs(right - left) <= 1);
    CuAssertIntEquals(tc, (left > right ? left : right) + 1, limit->height);

    for(order = limit->headOrder; order != NULL; order = order->nextOrder){
        CuAssertPtrEquals(tc, limit, order->parentLimit);
        if(order->nextOrder != NULL){
            CuAssertPtrEquals(tc, order, order->nextOrder->prevOrder);
        }
        else{
            CuAssertPtrEquals(tc, order, limit->tailOrder);
        }
        size += order->shares;
        orders++;
    }
    CuAssertTrue(tc, orders > 0);
    CuAssertIntEquals(tc, orders, limit->orderCount);
    CuAssertDblEquals(tc, size, limit->size, 1e-9);
    *count += 1;
    return limit->height;
}

static void
checkBook(CuTest *tc, Book *book){
    int bidLevels = 0, askLevels = 0;
    Limit *bids = book->buyTree->rightChild;
    Limit *asks = book->sellTree->rightChild;
    checkSubtree(tc, bids, book->buyTree, -INFINITY, INFINITY, &bidLevels);
    checkSubtree(tc, asks, book->sellTree, -INFINITY, INFINITY, &askLevels);
    CuAssertIntEquals(tc, bidLevels, (int)book->buyLimits.size);
    CuAssertIntEquals(tc, askLevels, (int)book->sellLimits.size);
    CuAssertPtrEquals(tc, bids ? getMaximumLimit(bids) : NULL, getBestBid(book));
    CuAssertPtrEquals(tc, asks ? getMinimumLimit(asks) : NULL, getBestAsk(book));
}

void
TestBookRandomOrderFlow(CuTest *tc){
    enum { OPS = 20000, MAX_LIVE = 4096 };
    Book *book = createBook();
    OrderId live[MAX_LIVE];
    int liveCount = 0;
    int op;
    srand(42);

    for(op = 0; op < OPS; ++op){
        int roll = rand() % 100;
        if(liveCount > 0 && roll < 8){
            /* execute: random side, size and limit, then drop filled ids */
            unsigned side = rand() % 2 ? HFTLOB_BUY : HFTLOB_SELL;
            double limitPrice = 1 + rand() % 120;
            double want = 1 + rand() % 40;
            double before = 0, after = 0;
            int i, kept = 0;
            Limit *best = side == HFTLOB_BUY ? getBestAsk(book) : getBestBid(book);
            if(best != NULL){
                before = best->size;
            }
            double filled = executeInBook(book, side, want, limitPrice, NULL, NULL);
            CuAssertTrue(tc, filled >= 0 && filled <= want);
            if(best != NULL && filled > 0 && filled < before){
                after = (side == HFTLOB_BUY ? getBestAsk(book) : getBestBid(book))->size;
                CuAssertDblEquals(tc, before - filled, after, 1e-9);
            }
            for(i = 0; i < liveCount; ++i){
                if(getOrderFromBook(book, live[i]) != NULL){
                    live[kept++] = live[i];
                }
            }
            liveCount = kept;
        }
        else if(liveCount > 0 && roll < 20){
            /* replace: new size and/or price */
            Order *order = getOrderFromBook(book, live[rand() % liveCount]);
            double price = (rand() % 60) + (order->buyOrSell == HFTLOB_BUY ? 1 : 61);
            CuAssertIntEquals(tc, 1, replaceOrderInBook(book, order->id, 1 + rand() % 10, price));
        }
        else if(liveCount > 0 && (roll < 58 || liveCount == MAX_LIVE)){
            /* cancel */
            int idx = rand() % liveCount;
            CuAssertIntEquals(tc, 1, removeOrderFromBook(book, live[idx]));
            live[idx] = live[--liveCount];
        }
        else{
            /* add */
            unsigned side = rand() % 2 ? HFTLOB_BUY : HFTLOB_SELL;
            double price = (rand() % 60) + (side == HFTLOB_BUY ? 1 : 61);
            CuAssertPtrNotNull(tc, addOrderToBook(book, (OrderId)op, side, 1 + rand() % 10, price));
            live[liveCount++] = (OrderId)op;
        }
        CuAssertIntEquals(tc, liveCount, (int)book->orders.size);
        if(op % 50 == 0){
            checkBook(tc, book);
        }
    }
    checkBook(tc, book);
    freeBook(book);
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
    SUITE_ADD_TEST(suite, TestBalanceBranch);
    SUITE_ADD_TEST(suite, TestQueueIsFifo);
    SUITE_ADD_TEST(suite, TestInitLimitClearsTail);
    SUITE_ADD_TEST(suite, TestRemoveLimitWithDeepSuccessor);
    SUITE_ADD_TEST(suite, TestMapBasics);
    SUITE_ADD_TEST(suite, TestMapRandomAgainstReference);
    SUITE_ADD_TEST(suite, TestPoolReuseAndGrowth);
    SUITE_ADD_TEST(suite, TestBookAddAndBestPrices);
    SUITE_ADD_TEST(suite, TestBookRejectsInvalidOrders);
    SUITE_ADD_TEST(suite, TestBookRemove);
    SUITE_ADD_TEST(suite, TestBookReplace);
    SUITE_ADD_TEST(suite, TestBookRotationsKeepParentLinks);
    SUITE_ADD_TEST(suite, TestBookExecuteBuy);
    SUITE_ADD_TEST(suite, TestBookExecuteSell);
    SUITE_ADD_TEST(suite, TestBookExecuteRejectsInvalidInput);
    SUITE_ADD_TEST(suite, TestBookRandomOrderFlow);

    return suite;
}

CuSuite* HFTLobGetSuite();

int RunAllTests(void){
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, HFTLobGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return suite->failCount;
}
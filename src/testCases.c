#include "cuTest.h"
#include "hftlob.h"



void
TestOrderPushing(CuTest *tc){
    Limit limit;
    Limit *ptr_limit = &limit;
    limit.limitPrice = 1000.0;


    /**
     * Push a single order to an empty limit and check that both tail and head point towards it.
     * Also check that the inserted Order has the ptr set in its Order.parent_limit attribute.
     */
    Order newOrderA;
    Order *ptr_newOrderA = &newOrderA;
    newOrderA.limit = 1000.0;
    newOrderA.shares = 10;
    newOrderA.buyOrSell = 0;
    newOrderA.tid = 1234;

    float expected_volume = 0.0;
    float expected_size = 0;
    int expected_orderCount = 0;

    pushOrder(ptr_limit, ptr_newOrderA);

    // Assert References have been correctly updated
    CuAssertPtrNotNull(tc, limit.headOrder);
    CuAssertPtrNotNull(tc, limit.TailOrder);
    CuAssertPtrEquals(tc, limit.headOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.TailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.tailOrder->parent_limit, ptr_limit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderA.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderA.size);
    CuAssertIntEquals(tc, limit.orderCount, 1);
    expected_size = limit.size;
    expected_volume = limit.totalVolume;
    expected_orderCount++;


    /**
     * Push another order and assert that the limit.headOrder is updated correctly, and the orders linked together
     * properly (order.nextOrder should be NULL for tail, and order.previousOrder should be NULL for head; the other
     * pointers should point to the respective order).
     */
    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.tid = 1235;

    pushOrder(ptr_limit, ptr_newOrderB);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, limit.headOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, limit.hailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.tailOrder->parent_limit, ptr_limit);
    CuAssertPtrEquals(tc, limit.headOrder->parent_limit, ptr_limit);
    CuAssertPtrEquals(tc, limit.headOrder->nextOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, limit.tailOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder, NULL);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderB.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderB.size);
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
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.tid = 1236;

    pushOrder(ptr_limit, ptr_newOrderC);

    // Assert References have been correctly updated
    CuAssertPtrEquals(tc, limit.headOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, limit.hailOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.tailOrder->parent_limit, ptr_limit);
    CuAssertPtrEquals(tc, limit.headOrder->parent_limit, ptr_limit);
    CuAssertPtrEquals(tc, limit.headOrder->nextOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, limit.headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, limit.tailOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder, NULL);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder->nextOrder, ptr_newOrderA);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder->prevOrder, ptr_newOrderB);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder->parentLimit, ptr_limit);
    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, expected_size + newOrderB.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, expected_size + newOrderB.size);
    CuAssertIntEquals(tc, limit.orderCount, 2);
    expected_size = limit.size;
    expected_volume = limit.totalVolume;
    expected_orderCount++;
}

void
TestOrderPopping(CuTest *tc){
    Limit limit;
    Limit *ptr_limit = &limit;
    limit.limitPrice = 1000.0;

    Order newOrderA;
    Order *ptr_newOrderA = &newOrderA;
    newOrderA.limit = 1000.0;
    newOrderA.shares = 10;
    newOrderA.buyOrSell = 0;
    newOrderA.tid = 1234;

    Order newOrderB;
    Order *ptr_newOrderB = &newOrderB;
    newOrderB.limit = 1000.0;
    newOrderB.shares = 20;
    newOrderB.buyOrSell = 0;
    newOrderB.tid = 1235;

    Order newOrderC;
    Order *ptr_newOrderC = &newOrderC;
    newOrderC.limit = 1000.0;
    newOrderC.shares = 30;
    newOrderC.buyOrSell = 0;
    newOrderC.tid = 1236;

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
    CuAssertPtrEquals(tc, limit.headOrder, newOrderC);
    CuAssertPtrEquals(tc, limit.tailOrder, newOrderB);
    CuAssertPtrEquals(tc, limit.tailOrder->prevOrder, ptr_newOrderC);
    CuAssertPtrEquals(tc, limit.tailOrder->nextOrder, NULL);
    CuAssertPtrEquals(tc, limit.headOrder->prevOrder, NULL);
    CuAssertPtrEquals(tc, limit.headOrder->nextOrder, ptr_newOrderB);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderA.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderA.size);
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
    CuAssertPtrEquals(tc, limit.headOrder, newOrderC);
    CuAssertPtrEquals(tc, limit.tailOrder, newOrderC);
    CuAssertPtrEquals(tc, newOrderC.prevOrder, NULL);
    CuAssertPtrEquals(tc, newOrderC.nextOrder, NULL);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderB.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderB.size);
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
    CuAssertPtrEquals(tc, limit.headOrder, NULL);
    CuAssertPtrEquals(tc, limit.tailOrder, NULL);

    // Assert that attributes in limit have been correctly updated
    CuAssertDblEquals(tc, limit.totalVolume, currentVolume - newOrderC.size * limit.limitPrice);
    CuAssertDblEquals(tc, limit.size, currentSize - newOrderC.size);
    CuAssertIntEquals(tc, limit.orderCount, 0);

    // // // //
    // Pop # 4
    // // // //
    isPopped = popOrder(ptr_limit);
    CuAssertIntEquals(tc, isPopped, 0);
}

void
TestCreateRoot(CuTest *tc){
    /**
     * Test the createRoot() function and assert it creates a root with limitPrice of -INFINITY, no children
     * and no parent.
     */
    Limit *ptr_root = createRoot();
    CuAssertPtrEquals(tc, ptr_root->parent, NULL);
    CuAssertPtrEquals(tc, ptr_root->limitPrice, -INFINITY);
    CuAssertPtrEquals(tc, ptr_root->leftChild, NULL);
    CuAssertPtrEquals(tc, ptr_root->rightChild, NULL);
}

void
TestAddNewLimit(CuTest *tc){
    /**
     * Assert that the addNewLimit() function adds a Limit structure dependent on it limitPrice into a
     * given root Limit structure.
     */
    Limit *ptr_root = createRoot();

    Limit newLimitA;
    Limit *ptr_newLimitA = &newLimit;
    newLimitA.limitPrice = 100;

    Limit newLimitB;
    Limit *ptr_newLimitB = &newLimitB;
    newLimitB.limitPrice = 50;

    Limit newLimitC;
    Limit *ptr_newLimitC = &newLimitC;
    newLimitC.limitPrice = 200;

    int statusCode = 0;

    /**
     * Add the first limit. Assert it is added as the leftChild of root. Check its references after addition to root.
     */
    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuTestAssertIntEquals(tc, statusCode, 1);

    CuTestAssertPtrEquals(tc, ptr_root->leftChild, ptr_newLimitA);
    CuTestAssertPtrEquals(tc, ptr_root->leftChild->parent, ptr_root);

    /**
      * Add the second limit. Assert it is added as root->leftChild->leftChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuTestAssertIntEquals(tc, statusCode, 1);

    CuTestAssertPtrEquals(tc, ptr_root->leftChild->leftChild, ptr_newLimitB);
    CuTestAssertPtrEquals(tc, ptr_root->leftChild->leftChild->parent, ptr_newLimitA);

    /**
      * Add the third limit. Assert it is added as root->leftChild->rightChild. Check references to parent.
      */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuTestAssertIntEquals(tc, statusCode, 1);

    CuTestAssertPtrEquals(tc, ptr_root->leftChild->rightChild, ptr_newLimitC);
    CuTestAssertPtrEquals(tc, ptr_root->leftChild->rightChild->parent, ptr_newLimitA);
    /**
     * Add a duplicate limit and assert the returned status code is 0.
     */
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuTestAssertIntEquals(tc, statusCode, 0);
}

void
TestRemoveLimit(CuTest *tc){
    /**
     * Assert that removeLimit() successfully removes a limit, no matter where this limit is present at.
     */
    Limit *ptr_root = createRoot();

    Limit newLimitA;
    Limit *ptr_newLimitA = &newLimit;
    newLimitA.limitPrice = 100;

    Limit newLimitB;
    Limit *ptr_newLimitB = &newLimitB;
    newLimitB.limitPrice = 50;

    Limit newLimitC;
    Limit *ptr_newLimitC = &newLimitC;
    newLimitC.limitPrice = 200;

    int statCode = 0;

    /**
     * TestCase1: Removing Limits which are present as children, with no children of their own.
     */

    // Set up the test Limit BST
    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuTestAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuTestAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuTestAssertIntEquals(tc, statusCode, 1);

    // Remove first child
    statCode = removeLimit(ptr_newLimitB);
    CuTestAssertIntEquals(tc, statusCode, 1);
    CuTestAssertPtrEquals(ptr_root->leftChild->leftChild, NULL);
    CuTestAssertPtrEquals(ptr_root->leftChild->rightChild, ptr_newLimitC);

    // Remove second child
    statCode = removeLimit(ptr_newLimitC);
    CuTestAssertIntEquals(tc, statusCode, 1);
    CuTestAssertPtrEquals(ptr_root->leftChild->rightChild, NULL);
    CuTestAssertPtrEquals(ptr_root->leftChild->leftChild, NULL);

    /**
     * TestCase2: Remove a limit which has a single child and parent
     */

    // Reset the test BST
    free(ptr_root);
    ptr_root = createRoot();
    statusCode = addNewLimit(ptr_root, ptr_newLimitA);
    CuTestAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitB);
    CuTestAssertIntEquals(tc, statusCode, 1);
    statusCode = addNewLimit(ptr_root, ptr_newLimitC);
    CuTestAssertIntEquals(tc, statusCode, 1);

    CuTestFail("Finish this test!");
}

CuSuite* HFTLobGetSuite(){
    /**
     * Prepare the test suite.
     */
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestOrderPushing);
    SUITE_ADD_TEST(suite, TestOrderPopping);
    return suite;
}
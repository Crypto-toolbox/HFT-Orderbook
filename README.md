# HFT-Orderbook
Limit Order Book for high-frequency trading (HFT), as described by WK Selph, implemented in Python3 (C implementation on the way)

Based on WK Selph's Blogpost:

http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/

Available at Archive.org's WayBackMachine:

https://goo.gl/KF1SRm


    "There are three main operations that a limit order book (LOB) has to
    implement: add, cancel, and execute.  The goal is to implement these
    operations in O(1) time while making it possible for the trading model to
    efficiently ask questions like “what are the best bid and offer?”, “how much
    volume is there between prices A and B?” or “what is order X’s current
    position in the book?”.

    The vast majority of the activity in a book is usually made up of add and
    cancel operations as market makers jockey for position, with executions a
    distant third (in fact I would argue that the bulk of the useful information
    on many stocks, particularly in the morning, is in the pattern of adds and
    cancels, not executions, but that is a topic for another post).  An add
    operation places an order at the end of a list of orders to be executed at
    a particular limit price, a cancel operation removes an order from anywhere
    in the book, and an execution removes an order from the inside of the book
    (the inside of the book is defined as the oldest buy order at the highest
    buying price and the oldest sell order at the lowest selling price).  Each
    of these operations is keyed off an id number (Order.idNumber in the
    pseudo-code below), making a hash table a natural structure for tracking
    them.

    Depending on the expected sparsity of the book (sparsity being the
    average distance in cents between limits that have volume, which is
    generally positively correlated with the instrument price), there are a
    number of slightly different implementations I’ve used.  First it will help
    to define a few objects:

        Order
          int idNumber;
          bool buyOrSell;
          int shares; // order size
          int limit;
          int entryTime;
          int eventTime;
          Order *nextOrder;
          Order *prevOrder;
          Limit *parentLimit;

        Limit  // representing a single limit price
          int limitPrice;
          int size;
          int totalVolume;
          Limit *parent;
          Limit *leftChild;
          Limit *rightChild;
          Order *headOrder;
          Order *tailOrder;

        Book
          Limit *buyTree;
          Limit *sellTree;
          Limit *lowestSell;
          Limit *highestBuy;

    The idea is to have a binary tree of Limit objects sorted by limitPrice,
    each of which is itself a doubly linked list of Order objects.  Each side
    of the book, the buy Limits and the sell Limits, should be in separate trees
    so that the inside of the book corresponds to the end and beginning of the
    buy Limit tree and sell Limit tree, respectively.  Each order is also an
    entry in a map keyed off idNumber, and each Limit is also an entry in a
    map keyed off limitPrice.

    With this structure you can easily implement these key operations with
    good performance:

    Add – O(log M) for the first order at a limit, O(1) for all others
    Cancel – O(1)
    Execute – O(1)
    GetVolumeAtLimit – O(1)
    GetBestBid/Offer – O(1)

    where M is the number of price Limits (generally << N the number of orders).
    Some strategy for keeping the limit tree balanced should be used because the
    nature of markets is such that orders will be being removed from one side
    of the tree as they’re being added to the other.  Keep in mind, though,
    that it is important to be able to update Book.lowestSell/highestBuy
    in O(1) time when a limit is deleted (which is why each Limit has a Limit
    *parent) so that GetBestBid/Offer can remain O(1)."

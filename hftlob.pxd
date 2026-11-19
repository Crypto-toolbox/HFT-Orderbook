from libc.stddef cimport size_t
from libc.stdint cimport uint64_t


cdef extern from "hftlob.h":
    ctypedef uint64_t OrderId

    ctypedef struct Order:
        OrderId id
        unsigned buyOrSell
        double shares
        double limit
        double entryTime
        double eventTime
        Order *nextOrder
        Order *prevOrder
        Limit *parentLimit
        int exchangeId

    ctypedef struct Limit:
        double limitPrice
        double size
        double totalVolume
        int orderCount
        int height
        Limit *parent
        Limit *leftChild
        Limit *rightChild
        Order *headOrder
        Order *tailOrder

    ctypedef struct Book:
        pass

    ctypedef struct LevelSnapshot:
        double price
        double size
        double totalVolume
        int orderCount

    ctypedef struct OrderSnapshot:
        OrderId uid
        unsigned buyOrSell
        double size
        double price
        double timestamp

    enum: HFTLOB_SELL
    enum: HFTLOB_BUY

    Book *createBook()
    void freeBook(Book *book)

    Order *addOrderToBook(Book *book, OrderId id, unsigned buyOrSell,
                          double shares, double price)
    int removeOrderFromBook(Book *book, OrderId id)
    int replaceOrderInBook(Book *book, OrderId id, double shares, double price)
    Order *getOrderFromBook(const Book *book, OrderId id)
    Limit *getLimitFromBook(const Book *book, unsigned buyOrSell, double price)
    Limit *getBestBid(const Book *book)
    Limit *getBestAsk(const Book *book)
    size_t getLevelsSnapshot(const Book *book, unsigned buyOrSell,
                             LevelSnapshot *out, size_t maxLevels)
    size_t getLevelOrderSnapshots(const Book *book, unsigned buyOrSell,
                                 double price, OrderSnapshot *out,
                                 size_t maxOrders)

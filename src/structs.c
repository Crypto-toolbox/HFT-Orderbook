struct Order{
    int tid;
    unsigned buyOrSell;
    int shares;
    int limit;
    int entryTime;
    int eventTime;
    struct Order *nextOrder;
    struct Order *prevOrder;
    struct Limit *parentLimit;
};

struct Limit{
    float limitPrice;
    float size;
    float totalVolume;
    int orderCount;
    struct Limit *parent;
    struct Limit *leftChild;
    struct Limit *rightChild;
    struct Order *headOrder;
    struct Order *tailOrder;
};

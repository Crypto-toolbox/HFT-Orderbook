struct Order
    int tid;
    bool buyOrSell;
    int shares; // order size
    int limit;
    int entryTime;
    int eventTime;
    Order *nextOrder;
    Order *prevOrder;
    Limit *parentLimit;
};

struct Limit{
    float limitPrice;
    float size;
    float totalVolume;
    int orderCount;
    Limit *parent;
    Limit *leftChild;
    Limit *rightChild;
    Order *headOrder;
    Order *tailOrder;
}

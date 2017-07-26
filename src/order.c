struct Order {
    int uid;
    bool is_bid;
    float size;
    float price;
    float timestamp;

    Order *next_order;
    Order *previous_order;
    LimitLevel **parent_limit;
};

struct OrderList{
    Order *head;
    Order *tail;
    int count;
    LimitLevel *parent_limit;

    void push(Order *new_order)
    /* Push function to add an Order to an OrderList struct at head*/
    {
        new_order->parent_limit = parent_limit;
        new_order->next = head;
        new_order->previous = NULL;

        // Check if our head is NULL (if the list is empty)
        if (head != NULL){
                // It isn't ! Push the new order
                head->previous = new_order;
        }
        else{
            // It is! Set the new Order as tail
            tail = new_order;
        };

        // update head and increment counter of list
        head = newEntry;
        count = count + 1;

        return;
    }

    void pop()
    /* Pop function to remove tail from an OrderList struct*/
    {
        // Check if our list is empty (tail is Null)
        if (tail == NULL){
            return;
        }
        // Pop the tail
        Order* newTail = tail->previous;
        if (tail->previous != NULL){
            tail->previous->next = NULL;
        }
        else{
            head = NULL;
        }
        // Update tail and decrement counter of list
        tail = newTail;
        count = count - 1;

        return;
    }
};
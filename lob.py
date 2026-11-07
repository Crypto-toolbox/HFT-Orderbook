"""
MIT License

Copyright (c) 2017 Nils Diefenbach

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

--------------------------------------------------------------------------------

HFT-Orderbook

Limit Order Book for high-frequency trading (HFT), as described by WK Selph,
implemented in Python3.

Based on WK Selph's Blogpost:
http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/

Available at Archive.org's WayBackMachine:
(https://goo.gl/KF1SRm)


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

"""

# Import Built-Ins
import logging
import time
from itertools import islice
# Import Third-Party

# Import Homebrew


# Init Logging Facilities
log = logging.getLogger(__name__)


class LimitOrderBook:
    """Limit Order Book (LOB) implementation for High Frequency Trading

    Implementation as described by WK Selph (see header doc string for link).

    """
    def __init__(self):
        self.bids = LimitLevelTree()
        self.asks = LimitLevelTree()
        self.best_bid = None
        self.best_ask = None
        self._price_levels = {}
        self._orders = {}

    @property
    def top_level(self):
        """Returns the best available bid and ask.

        :return:
        """
        return self.best_bid, self.best_ask

    def process(self, order):
        """Processes the given order.

        If the order's size is 0, it is removed from the book.

        If its size isn't zero and it exists within the book, the order is
        updated (a price or side change moves it to its new price level).

        If it doesn't exist, it will be added.

        :param order:
        :return:
        """
        if order.size == 0:
            self.remove(order)
        else:
            try:
                self.update(order)
            except KeyError:
                self.add(order)

    def update(self, order):
        """Updates an existing order in the book.

        A size-only change is applied in place, keeping the order's time
        priority, and the related LimitLevel's size is adjusted accordingly.

        A change of price or side moves the order: it is removed and re-added
        at the back of its new price level, losing its time priority (as it
        would on an exchange).

        :param order:
        :return:
        :raises KeyError: if the order is not in the book.
        """
        existing = self._orders[order.uid]
        if existing.price != order.price or existing.is_bid != order.is_bid:
            self.remove(existing)
            self.add(order)
            return

        size_diff = existing.size - order.size
        self._orders[order.uid].size = order.size
        self._orders[order.uid].parent_limit.size -= size_diff

    def remove(self, order):
        """Removes an order from the book.

        If the Limit Level is then empty, it is also removed from the book's
        relevant tree.

        If the removed LimitLevel was either the top bid or ask, it is replaced
        by the next best price level on that side (its in-order neighbour in
        the AVL tree).

        :param order:
        :return:
        """
        # Remove Order from self._orders
        try:
            popped_item = self._orders.pop(order.uid)
        except KeyError:
            return False

        # Remove order from its doubly linked list
        popped_item.pop_from_list()

        # Remove Limit Level from self._price_levels and tree, if no orders are
        # left within that limit level
        try:
            if len(self._price_levels[popped_item.price]) == 0:
                popped_limit_level = self._price_levels.pop(popped_item.price)
                # Remove Limit Level from LimitLevelTree
                if popped_item.is_bid:
                    if popped_limit_level is self.best_bid:
                        self.best_bid = popped_limit_level.predecessor
                else:
                    if popped_limit_level is self.best_ask:
                        self.best_ask = popped_limit_level.successor
                popped_limit_level.remove()
        except KeyError:
            pass

        return popped_item

    def add(self, order):
        """Adds a new LimitLevel to the book and appends the given order to it.

        :param order: Order() Instance
        :return:
        """

        if order.price not in self._price_levels:
            limit_level = LimitLevel(order)
            self._orders[order.uid] = order
            self._price_levels[limit_level.price] = limit_level

            if order.is_bid:
                self.bids.insert(limit_level)
                if self.best_bid is None or limit_level.price > self.best_bid.price:
                    self.best_bid = limit_level

            else:
                self.asks.insert(limit_level)
                if self.best_ask is None or limit_level.price < self.best_ask.price:
                    self.best_ask = limit_level
        else:
            # The price level already exists, hence we need to append the order
            # to that price level
            self._orders[order.uid] = order
            self._price_levels[order.price].append(order)
    
    
    def levels(self, depth=None):
        """Returns the price levels as a dict {'bids': [bid1, ...], 'asks': [ask1, ...]}
        
        :param depth: Desired number of levels on each side to return.
        :return:
        """
        levels_sorted = sorted(self._price_levels.items())
        bids_all = reversed([price for price, level in levels_sorted
                             if level.orders.head.is_bid])
        asks_all = (price for price, level in levels_sorted
                    if not level.orders.head.is_bid)
        bids = list(islice(bids_all, depth)) if depth else list(bids_all)
        asks = list(islice(asks_all, depth)) if depth else list(asks_all)
        levels_dict = {
            'bids' : [self._price_levels[price] for price in bids],
            'asks' : [self._price_levels[price] for price in asks],
            }
        return levels_dict

class LimitLevel:
    """AVL BST node.

    This Binary Tree implementation balances on each insert.

    If performance is of concern to you, implementing a bulk-balance
    method may be of interest (c-based implementations aside).

    Attributes:
        parent: Parent node of this Node
        is_root: Boolean, True if this Node is the topmost node of its tree
        left_child: Left child of this Node; Values smaller than price
        right_child: Right child of this Node; Values greater than price

    Properties:
        height: Height of this Node
        balance: Balance factor of this Node
    """
    __slots__ = ['price', 'size', 'parent', 'left_child',
                 'right_child', 'orders', 'height']

    def __init__(self, order):
        """Initialize a Node() instance.

        :param order:
        """
        # Data Values
        self.price = order.price
        self.size = order.size

        # BST Attributes
        self.parent = None
        self.left_child = None
        self.right_child = None
        self.height = 1  # cached; kept up to date by balance() and rotations

        # Doubly-Linked-list attributes
        self.orders = OrderList(self)
        self.append(order)

    @property
    def is_root(self):
        return isinstance(self.parent, LimitLevelTree)

    @property
    def volume(self):
        return self.price * self.size

    @property
    def balance_factor(self):
        """Calculate and return the balance of this Node.

        Calculate balance by dividing the right child's height from
        the left child's height. Children which evaluate to False (None)
        are treated as zeros.
        :return:
        """
        right_height = self.right_child.height if self.right_child else 0
        left_height = self.left_child.height if self.left_child else 0

        return right_height - left_height

    @property
    def grandpa(self):
        try:
            if self.parent:
                return self.parent.parent
            else:
                return None
        except AttributeError:
            return None

    def _update_height(self):
        """Recompute the cached height from our children's cached heights."""
        left_height = self.left_child.height if self.left_child else 0
        right_height = self.right_child.height if self.right_child else 0
        self.height = max(left_height, right_height) + 1

    @property
    def min(self):
        """Returns the smallest node under this node.

        :return:
        """
        minimum = self
        while minimum.left_child:
            minimum = minimum.left_child
        return minimum

    @property
    def max(self):
        """Returns the largest node under this node.

        :return:
        """
        maximum = self
        while maximum.right_child:
            maximum = maximum.right_child
        return maximum

    @property
    def predecessor(self):
        """Returns the next lower LimitLevel in the tree, or None."""
        if self.left_child:
            return self.left_child.max
        node = self
        while not node.is_root and node is node.parent.left_child:
            node = node.parent
        return None if node.is_root else node.parent

    @property
    def successor(self):
        """Returns the next higher LimitLevel in the tree, or None."""
        if self.right_child:
            return self.right_child.min
        node = self
        while not node.is_root and node is node.parent.right_child:
            node = node.parent
        return None if node.is_root else node.parent

    def append(self, order):
        """Wrapper function to make appending to Order List simpler.

        :param order: Order() Instance
        :return:
        """
        return self.orders.append(order)

    def _replace_node_in_parent(self, new_value=None):
        """Replaces Node in parent on a delete() call.

        :param new_value: LimitLevel() instance
        :return:
        """
        if not self.is_root and self is self.parent.left_child:
            self.parent.left_child = new_value
        else:
            self.parent.right_child = new_value
        if new_value:
            new_value.parent = self.parent

    def remove(self):
        """Deletes this limit level.

        :return:
        """

        if self.left_child and self.right_child:
            # We have two kids: splice in our in-order successor.
            succ = self.right_child.min
            if succ is not self.right_child:
                rebalance_from = succ.parent
                succ._replace_node_in_parent(succ.right_child)
                succ.right_child = self.right_child
                succ.right_child.parent = succ
            else:
                rebalance_from = succ
            self._replace_node_in_parent(succ)
            succ.left_child = self.left_child
            succ.left_child.parent = succ
        else:
            rebalance_from = self.parent
            self._replace_node_in_parent(self.left_child or self.right_child)

        self.parent = self.left_child = self.right_child = None
        if not isinstance(rebalance_from, LimitLevelTree):
            rebalance_from.balance()

    def balance_parent(self):
        """Rebalance (and refresh heights) from our parent upwards.

        :return:
        """
        if isinstance(self.parent, LimitLevel):
            self.parent.balance()

    def balance(self):
        """Call the rotation method relevant to this Node's balance factor.

        This call works itself up the tree, refreshing cached heights on the
        way, so it must be called on the lowest node whose subtree changed.

        :return:
        """
        self._update_height()
        if self.balance_factor > 1:
            # right is heavier
            if self.right_child.balance_factor < 0:
                # right_child.left is heavier, RL case
                self._rl_case()
            else:
                # right_child.right is heavier (or even), RR case
                self._rr_case()
        elif self.balance_factor < -1:
            # left is heavier
            if self.left_child.balance_factor > 0:
                # left_child.right is heavier, LR case
                self._lr_case()
            else:
                # left_child.left is heavier (or even), LL case
                self._ll_case()
        else:
            # Everything's fine.
            pass

        # Now check upwards
        if isinstance(self.parent, LimitLevel):
            self.parent.balance()

    def _rotate_left(self):
        """Rotate left around this node; our right child takes our place."""
        child = self.right_child
        self._replace_node_in_parent(child)
        self.right_child = child.left_child
        if self.right_child:
            self.right_child.parent = self
        child.left_child = self
        self.parent = child
        self._update_height()
        child._update_height()

    def _rotate_right(self):
        """Rotate right around this node; our left child takes our place."""
        child = self.left_child
        self._replace_node_in_parent(child)
        self.left_child = child.right_child
        if self.left_child:
            self.left_child.parent = self
        child.right_child = self
        self.parent = child
        self._update_height()
        child._update_height()

    def _ll_case(self):
        """Rotate Nodes for LL Case.

        Reference:
            https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
        :return:
        """
        self._rotate_right()

    def _rr_case(self):
        """Rotate Nodes for RR Case.

        Reference:
            https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
        :return:
        """
        self._rotate_left()

    def _lr_case(self):
        """Rotate Nodes for LR Case.

        Reference:
            https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
        :return:
        """
        self.left_child._rotate_left()
        self._rotate_right()

    def _rl_case(self):
        """Rotate Nodes for RL case.

        Reference:
            https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
        :return:
        """
        self.right_child._rotate_right()
        self._rotate_left()

    def __str__(self):
        s = 'Node Value: %s\n' % self.price
        s += 'Node left_child value: %s\n' % (self.left_child.price if self.left_child else 'None')
        s += 'Node right_child value: %s\n\n' % (self.right_child.price if self.right_child else 'None')

        left_side_print = self.left_child.__str__() if self.left_child else ''
        right_side_print = self.right_child.__str__() if self.right_child else ''
        return s + left_side_print + right_side_print

    def __len__(self):
        return len(self.orders)


class LimitLevelTree:
    """AVL BST Root Node.

    """
    __slots__ = ['right_child', 'is_root']

    def __init__(self):
        # BST Attributes
        self.right_child = None
        self.is_root = True

    def insert(self, limit_level):
        """Iterative AVL Insert method to insert a new Node.

        Inserts a new node, then rebalances from its parent upwards.

        :param limit_level:
        :return:
        """
        current_node = self
        while True:
            if current_node is self or limit_level.price > current_node.price:
                if current_node.right_child is None:
                    current_node.right_child = limit_level
                    current_node.right_child.parent = current_node
                    limit_level.balance_parent()
                    break
                else:
                    current_node = current_node.right_child
                    continue
            elif limit_level.price < current_node.price:
                if current_node.left_child is None:
                    current_node.left_child = limit_level
                    current_node.left_child.parent = current_node
                    limit_level.balance_parent()
                    break
                else:
                    current_node = current_node.left_child
                    continue
            else:
                # The level already exists
                break


class OrderList:
    """Doubly-Linked List Container Class.

    Stores head and tail orders, as well as count.

    Keeps a reference to its parent LimitLevel Instance.

    This container was added because it makes deleting the LimitLevels easier.

    Has no other functionality.

    """
    __slots__ = ['head', 'tail', 'parent_limit', 'count']

    def __init__(self, parent_limit):
        self.head = None
        self.tail = None
        self.count = 0
        self.parent_limit = parent_limit

    def __len__(self):
        return self.count

    def append(self, order):
        """Appends an order to this List.

        Same as LimitLevel append, except it automatically updates head and tail
        if it's the first order in this list.

        :param order:
        :return:
        """
        if not self.tail:
            order.root = self
            self.tail = order
            self.head = order
            self.count += 1
        else:
            self.tail.append(order)


class Order:
    """Doubly-Linked List Order item.

    Keeps a reference to root, as well as previous and next order in line.

    It also performs any and all updates to the root's tail, head and count
    references, as well as updating the related LimitLevel's size, whenever
    a method is called on this instance.

    Offers append() and pop() methods. Prepending isn't implemented.

    """
    __slots__ = ['uid', 'is_bid', 'size', 'price', 'timestamp',
                 'next_item', 'previous_item', 'root']

    def __init__(self, uid, is_bid, size, price, root=None,
                 timestamp=None, next_item=None, previous_item=None):
        # Data Values
        self.uid = uid
        self.is_bid = is_bid
        self.price = price
        self.size = size
        self.timestamp = timestamp if timestamp else time.time()

        # DLL Attributes
        self.next_item = next_item
        self.previous_item = previous_item
        self.root = root

    @property
    def parent_limit(self):
        return self.root.parent_limit

    def append(self, order):
        """Append an order.

        :param order: Order() instance
        :return:
        """
        if self.next_item is None:
            self.next_item = order
            self.next_item.previous_item = self
            self.next_item.root = self.root

            # Update Root Statistics in OrderList root obj
            self.root.count += 1
            self.root.tail = order

            self.parent_limit.size += order.size

        else:
            self.next_item.append(order)

    def pop_from_list(self):
        """Pops this item from the DoublyLinkedList it belongs to.

        :return: Order() instance values as tuple
        """
        if self.previous_item is None:
            # We're head
            self.root.head = self.next_item
        else:
            self.previous_item.next_item = self.next_item

        if self.next_item is None:
            # We're tail
            self.root.tail = self.previous_item
        else:
            self.next_item.previous_item = self.previous_item

        self.previous_item = self.next_item = None

        # Update the Limit Level and root
        self.root.count -= 1
        self.parent_limit.size -= self.size

        return self.__repr__()

    def __str__(self):
        return self.__repr__()

    def __repr__(self):
        return str((self.uid, self.is_bid, self.price, self.size, self.timestamp))

# Import Built-Ins
import logging
import random
from unittest import TestCase

# Import Third-Party

# Import Homebrew
from lob import LimitOrderBook, Order

# Init Logging Facilities
log = logging.getLogger(__name__)


class OrderTests(TestCase):

    def test_adding_a_new_order_works(self):
        lob = LimitOrderBook()
        bid_order = Order(uid=1, is_bid=True, size=5, price=100)
        ask_order = Order(uid=2, is_bid=False, size=5, price=200)
        lob.process(bid_order)
        lob.process(ask_order)
        self.assertEqual(lob.best_ask.price, 200)
        self.assertEqual(lob.best_bid.price, 100)
        self.assertEqual(lob.best_bid.volume, 500)

        # Assert that the best bid (bid_order) has no previous and no next item,
        # since it is the only one in the book on the bid size at the moment.
        self.assertEqual(len(lob.best_bid), 1)
        self.assertEqual(len(lob.best_ask), 1)
        self.assertIsNone(bid_order.next_item)
        self.assertIsNone(bid_order.previous_item)
        self.assertEqual(lob.best_bid.orders.head, bid_order)
        self.assertEqual(lob.best_ask.orders.head, ask_order)
        self.assertIn(1, lob._orders)

        # Assert that updating an order works
        updated_bid_order = Order(uid=1, is_bid=True, size=4, price=100, timestamp=bid_order.timestamp)
        lob.process(updated_bid_order)
        self.assertEqual(lob.best_bid.orders.head.size, 4)
        self.assertEqual(lob.best_bid.volume, 400)

        updated_ask_order = Order(uid=2, is_bid=False, size=4, price=200, timestamp=ask_order.timestamp)
        lob.process(updated_ask_order)
        self.assertEqual(lob.best_ask.orders.head.size, 4)
        self.assertEqual(lob.best_ask.volume, 800)

        # Assert that adding an additional order to a limit level updates the
        # doubly linked list correctly
        bid_order_2 = Order(uid=3, is_bid=True, size=5, price=100)
        lob.process(bid_order_2)
        self.assertEqual(lob.best_bid.orders.head.next_item, bid_order_2)
        self.assertEqual(lob.best_bid.orders.tail, bid_order_2)
        self.assertEqual(len(lob.best_bid), 2)
        
    def test_removing_orders_works(self):
        lob = LimitOrderBook()
        bid_order = Order(uid=1, is_bid=True, size=5, price=100)
        bid_order_2 = Order(uid=2, is_bid=True, size=10, price=100)
        ask_order = Order(uid=3, is_bid=False, size=10, price=200)
        ask_order_2 = Order(uid=4, is_bid=False, size=10, price=200)
        lob.process(bid_order)
        lob.process(bid_order_2)
        lob.process(ask_order)
        lob.process(ask_order_2)

        # Assert that removing an order from a limit level with several
        # orders resets the tail, head and previous / next items accordingly
        removed_bid_order = Order(uid=1, is_bid=True, size=0, price=100)
        self.assertEqual(len(lob.best_bid), 2)
        self.assertEqual(lob.best_bid.orders.head, bid_order)
        self.assertEqual(lob.best_bid.orders.tail, bid_order_2)
        lob.process(removed_bid_order)
        self.assertEqual(len(lob.best_bid), 1)
        self.assertEqual(lob.best_bid.orders.head, bid_order_2)
        self.assertEqual(lob.best_bid.orders.tail, bid_order_2)
        self.assertIsNone(lob.best_bid.orders.head.next_item)
        self.assertIsNone(lob.best_bid.orders.head.previous_item)
        self.assertNotIn(removed_bid_order.uid, lob._orders)
        self.assertIn(removed_bid_order.price, lob._price_levels)

        # Assert that removing the last Order in a price level removes its
        # limit Level accordingly
        removed_bid_order_2 = Order(uid=2, is_bid=True, size=0, price=100)
        lob.process(removed_bid_order_2)
        self.assertIsNone(lob.best_bid)

        self.assertNotIn(removed_bid_order_2.uid, lob._orders)
        self.assertNotIn(removed_bid_order_2.price, lob._price_levels)
    
    def load_book(self, lob):
        orders = [
            Order(uid=1, is_bid=True, size=5, price=100),
            Order(uid=2, is_bid=True, size=5, price=95),
            Order(uid=3, is_bid=True, size=5, price=90),
            Order(uid=4, is_bid=False, size=5, price=200),
            Order(uid=5, is_bid=False, size=5, price=205),
            Order(uid=6, is_bid=False, size=5, price=210),
            ]
        for order in orders:
             lob.process(order)
    
    def check_levels_format(self, levels):
        self.assertIsInstance(levels, dict)
        for side in ('bids', 'asks'):
            self.assertIsInstance(levels[side], list)
            for i, price_level in enumerate(levels[side]):
                price = price_level.price
                last_price = price if i < 1 else levels[side][i - 1].price
                if side == 'bids':
                    self.assertTrue(price <= last_price)
                else:
                    self.assertTrue(price >= last_price)
        
    def test_querying_levels_works(self):
        lob = LimitOrderBook()
        self.load_book(lob)
        levels = lob.levels()
        self.check_levels_format(levels)
    
    def test_querying_levels_limit_depth(self):
        lob = LimitOrderBook()
        self.load_book(lob)
        levels = lob.levels(depth=2)
        self.check_levels_format(levels)
        for side in ('bids', 'asks'):
            self.assertEqual(len(levels[side]), 2)
        


class RegressionTests(TestCase):

    def test_cancelling_a_middle_order_relinks_neighbours(self):
        lob = LimitOrderBook()
        for uid in (1, 2, 3):
            lob.process(Order(uid=uid, is_bid=True, size=1, price=100))
        lob.process(Order(uid=2, is_bid=True, size=0, price=100))

        head = lob.best_bid.orders.head
        self.assertEqual([head.uid, head.next_item.uid], [1, 3])
        self.assertIs(head.next_item.previous_item, head)
        self.assertEqual(len(lob.best_bid), 2)
        self.assertEqual(lob.best_bid.size, 2)

    def test_best_bid_falls_back_to_next_best_price(self):
        lob = LimitOrderBook()
        for uid, price in enumerate([100, 110, 105]):
            lob.process(Order(uid=uid, is_bid=True, size=1, price=price))
        lob.process(Order(uid=1, is_bid=True, size=0, price=110))
        self.assertEqual(lob.best_bid.price, 105)

    def test_best_ask_falls_back_to_next_best_price(self):
        lob = LimitOrderBook()
        for uid, price in enumerate([100, 90, 95]):
            lob.process(Order(uid=uid, is_bid=False, size=1, price=price))
        lob.process(Order(uid=1, is_bid=False, size=0, price=90))
        self.assertEqual(lob.best_ask.price, 95)

    def test_levels_with_one_empty_side(self):
        lob = LimitOrderBook()
        lob.process(Order(uid=1, is_bid=True, size=1, price=100))
        levels = lob.levels()
        self.assertEqual([lvl.price for lvl in levels['bids']], [100])
        self.assertEqual(levels['asks'], [])

    def test_removing_a_level_with_two_children_keeps_tree_ordered(self):
        lob = LimitOrderBook()
        for uid, price in enumerate([100, 50, 150, 120, 200]):
            lob.process(Order(uid=uid, is_bid=False, size=1, price=price))
        lob.process(Order(uid=0, is_bid=False, size=0, price=100))
        self.assertEqual(_in_order(lob.asks.right_child), [50, 120, 150, 200])


    def test_price_change_moves_order_to_new_level(self):
        lob = LimitOrderBook()
        lob.process(Order(uid=1, is_bid=True, size=5, price=100))
        lob.process(Order(uid=2, is_bid=True, size=3, price=105))
        lob.process(Order(uid=3, is_bid=True, size=4, price=105))

        # Move uid 1 up to 105: it joins the back of the queue there.
        lob.process(Order(uid=1, is_bid=True, size=5, price=105))
        self.assertNotIn(100, lob._price_levels)
        self.assertEqual(lob.best_bid.price, 105)
        self.assertEqual(lob.best_bid.size, 12)
        self.assertIs(lob.best_bid.orders.tail, lob._orders[1])

        # Move uid 2 down to 99: best bid stays, sizes follow.
        lob.process(Order(uid=2, is_bid=True, size=3, price=99))
        self.assertEqual(lob.best_bid.price, 105)
        self.assertEqual(lob.best_bid.size, 9)
        self.assertEqual(lob._price_levels[99].size, 3)

    def test_side_change_moves_order_to_other_tree(self):
        lob = LimitOrderBook()
        lob.process(Order(uid=1, is_bid=True, size=5, price=100))
        lob.process(Order(uid=1, is_bid=False, size=5, price=100))
        self.assertIsNone(lob.best_bid)
        self.assertEqual(lob.best_ask.price, 100)


def _in_order(node):
    if node is None:
        return []
    return _in_order(node.left_child) + [node.price] + _in_order(node.right_child)


class RandomisedInvariantTests(TestCase):
    """Hammer the book with random adds/cancels and check its invariants."""

    def _check_tree(self, tree):
        def check(node, parent):
            if node is None:
                return 0
            self.assertIs(node.parent, parent)
            left = check(node.left_child, node)
            right = check(node.right_child, node)
            self.assertLessEqual(abs(right - left), 1, 'AVL balance violated')
            self.assertEqual(node.height, max(left, right) + 1, 'stale cached height')
            return max(left, right) + 1

        check(tree.right_child, tree)
        prices = _in_order(tree.right_child)
        self.assertEqual(prices, sorted(set(prices)))
        return prices

    def test_random_order_flow(self):
        rng = random.Random(42)
        lob = LimitOrderBook()
        live = {}
        for uid in range(5000):
            if live and rng.random() < 0.15:
                move_uid = rng.choice(list(live))
                is_bid, _ = live[move_uid]
                price = rng.randint(1, 60) + (0 if is_bid else 60)
                lob.process(Order(uid=move_uid, is_bid=is_bid, size=rng.randint(1, 10), price=price))
                live[move_uid] = (is_bid, price)
            elif live and rng.random() < 0.45:
                cancel_uid = rng.choice(list(live))
                is_bid, price = live.pop(cancel_uid)
                lob.process(Order(uid=cancel_uid, is_bid=is_bid, size=0, price=price))
            else:
                is_bid = rng.random() < 0.5
                price = rng.randint(1, 60) + (0 if is_bid else 60)
                lob.process(Order(uid=uid, is_bid=is_bid, size=rng.randint(1, 10), price=price))
                live[uid] = (is_bid, price)

            bids = self._check_tree(lob.bids)
            asks = self._check_tree(lob.asks)
            self.assertEqual(lob.best_bid.price if lob.best_bid else None,
                             bids[-1] if bids else None)
            self.assertEqual(lob.best_ask.price if lob.best_ask else None,
                             asks[0] if asks else None)
            self.assertEqual(len(lob._orders), len(live))

# Import Built-Ins
import logging
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

        updated_ask_order = Order(uid=2, is_bid=True, size=4, price=200, timestamp=ask_order.timestamp)
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
        

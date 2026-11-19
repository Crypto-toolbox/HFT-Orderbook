import math
import unittest

from lob import LimitOrderBook, Order


class CythonOrderBookBehaviorTests(unittest.TestCase):
    """Behavioral tests for the C-backed implementation.

    The public API intentionally does not expose raw C pointers or AVL internals.
    Prices, sizes, and UIDs must therefore be validated at the Python boundary and
    the resulting behavior must be observed through the public book API.

    Expected numeric behavior: finite numeric values only; NaN and infinity are
    rejected because the C book and Python validation treat them as invalid input.
    """

    def test_adds_bid_and_ask_and_exposes_best_levels(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=5, price=100))
        book.process(Order(uid=2, is_bid=False, size=3, price=101))

        self.assertIsNotNone(book.best_bid)
        self.assertIsNotNone(book.best_ask)
        self.assertEqual(book.best_bid.price, 100)
        self.assertEqual(book.best_ask.price, 101)
        self.assertEqual(book.top_level, (book.best_bid, book.best_ask))

    def test_level_enumeration_is_sorted_and_depth_limited(self):
        book = LimitOrderBook()
        for uid, price in enumerate((95, 100, 105), start=1):
            book.process(Order(uid=uid, is_bid=True, size=2, price=price))
        for uid, price in enumerate((200, 205, 210), start=10):
            book.process(Order(uid=uid, is_bid=False, size=2, price=price))

        all_levels = book.levels()
        self.assertEqual([level.price for level in all_levels["bids"]], [105, 100, 95])
        self.assertEqual([level.price for level in all_levels["asks"]], [200, 205, 210])

        limited = book.levels(depth=2)
        self.assertEqual([level.price for level in limited["bids"]], [105, 100])
        self.assertEqual([level.price for level in limited["asks"]], [200, 205])

        empty = LimitOrderBook()
        self.assertEqual(empty.levels(), {"bids": [], "asks": []})
        self.assertEqual(empty.levels(depth=0), {"bids": [], "asks": []})

    def test_same_price_update_keeps_queue_priority(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=5, price=100))
        book.process(Order(uid=2, is_bid=True, size=7, price=100))
        book.process(Order(uid=1, is_bid=True, size=2, price=100))

        self.assertEqual(book.best_bid.price, 100)
        self.assertEqual(book.best_bid.size, 9)
        self.assertEqual(book.best_bid.order_count, 2)

    def test_price_change_requeues_order_at_new_level(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=5, price=100))
        book.process(Order(uid=2, is_bid=True, size=3, price=105))
        book.process(Order(uid=1, is_bid=True, size=5, price=105))

        self.assertEqual(book.best_bid.price, 105)
        self.assertEqual(book.best_bid.size, 8)

    def test_side_change_moves_order_to_opposite_side(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=5, price=100))
        book.process(Order(uid=1, is_bid=False, size=5, price=100))

        self.assertIsNone(book.best_bid)
        self.assertIsNotNone(book.best_ask)
        self.assertEqual(book.best_ask.price, 100)

    def test_zero_size_cancels_order_and_removes_empty_level(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=5, price=100))
        book.process(Order(uid=1, is_bid=True, size=0, price=100))

        self.assertIsNone(book.best_bid)
        self.assertEqual(book.levels(), {"bids": [], "asks": []})

    def test_bool_uids_are_rejected_as_non_integer_ids(self):
        with self.assertRaises(TypeError):
            Order(uid=True, is_bid=True, size=1, price=100)
        with self.assertRaises(TypeError):
            Order(uid=False, is_bid=False, size=1, price=100)

    def test_add_rejects_duplicate_uids(self):
        book = LimitOrderBook()
        try:
            book.add(Order(uid=11, is_bid=True, size=2, price=100))

            with self.assertRaises(ValueError):
                book.add(Order(uid=11, is_bid=True, size=3, price=101))
        finally:
            book.close()

    def test_process_updates_existing_uid(self):
        book = LimitOrderBook()
        try:
            book.process(Order(uid=11, is_bid=True, size=2, price=100))
            book.process(Order(uid=11, is_bid=True, size=3, price=101))

            order = book._book.get_order(11)
            self.assertIsNotNone(order)
            self.assertEqual(order["price"], 101)
            self.assertEqual(order["size"], 3)
        finally:
            book.close()

    def test_nan_and_infinity_are_rejected(self):
        book = LimitOrderBook()

        with self.assertRaises(ValueError):
            Order(uid=1, is_bid=True, size=1, price=float("nan"))
        with self.assertRaises(ValueError):
            Order(uid=1, is_bid=True, size=float("nan"), price=100)
        with self.assertRaises(ValueError):
            Order(uid=1, is_bid=True, size=float("inf"), price=100)
        with self.assertRaises(ValueError):
            Order(uid=1, is_bid=True, size=1, price=float("inf"))

        with self.assertRaises(ValueError):
            book.process(Order(uid=2, is_bid=True, size=1, price=float("nan")))

    def test_closed_book_rejects_new_operations(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=3, price=50))
        book.close()

        with self.assertRaisesRegex(RuntimeError, "book is closed"):
            book.process(Order(uid=2, is_bid=True, size=2, price=55))
        with self.assertRaisesRegex(RuntimeError, "book is closed"):
            book.add(Order(uid=3, is_bid=False, size=2, price=60))
        with self.assertRaisesRegex(RuntimeError, "book is closed"):
            book.levels()
        self.assertIsNone(book.best_bid)
        self.assertIsNone(book.best_ask)

    def test_snapshots_are_independent_from_underlying_book_state(self):
        book = LimitOrderBook()
        book.process(Order(uid=1, is_bid=True, size=6, price=100))
        snapshot = book.best_bid
        self.assertIsNotNone(snapshot)

        book.process(Order(uid=2, is_bid=True, size=4, price=105))
        self.assertEqual(snapshot.price, 100)
        self.assertEqual(snapshot.size, 6)
        self.assertEqual(book.best_bid.price, 105)

        levels = book.levels()
        first_bid = levels["bids"][0]
        book.process(Order(uid=1, is_bid=True, size=0, price=100))
        self.assertEqual(first_bid.price, 105)

    def test_invalid_uid_ranges_rejected(self):
        with self.assertRaises(ValueError):
            Order(uid=-1, is_bid=True, size=1, price=100)
        with self.assertRaises(ValueError):
            Order(uid=(1 << 64), is_bid=True, size=1, price=100)

    def test_repeated_close_cycle_is_safe(self):
        for _ in range(3):
            book = LimitOrderBook()
            book.process(Order(uid=1, is_bid=True, size=3, price=50))
            book.close()
            self.assertIsNone(book.best_bid)
            self.assertIsNone(book.best_ask)


if __name__ == "__main__":
    unittest.main()

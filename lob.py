"""MIT License

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
"""

"""HFT-Orderbook
Limit Order Book for high-frequency trading (HFT), as described by WK Selph, implemented in C, Python and Rust.

Three flavours live side by side:

- **C** (`src/`): a `Book` API on top of AVL limit trees, order queues and hash maps.
- **Python** (`lob.py`): a complete, pure-Python `LimitOrderBook`.
- **Rust** (`rust/`): a separate crate with the same operations, using integer prices and slab storage. See [`rust/README.md`](rust/README.md).

## Background

Based on WK Selph's Blogpost:

http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/

Available at Archive.org's WayBackMachine:

https://goo.gl/KF1SRm

    "There are three main operations that a limit order book (LOB) has to
    implement: add, cancel, and execute. The goal is to implement these
    operations in O(1) time while making it possible for the trading model to
    efficiently ask questions like “what are the best bid and offer?”, “how much
    volume is there between prices A and B?” or “what is order X’s current
    position in the book?”.

    The vast majority of the activity in a book is usually made up of add and
    cancel operations as market makers jockey for position, with executions a
    distant third (in fact I would argue that the bulk of the useful information
    on many stocks, particularly in the morning, is in the pattern of adds and
    cancels, not executions, but that is a topic for another post)."
"""

"""This Python façade is a Cython-backed wrapper around the C order-book engine.
It exposes snapshot-based Python views of the live C book and keeps the C book as
its single source of truth.

The legacy private AVL internals are no longer exposed: bids and asks are snapshot lists,
not live tree roots, and the old _orders and _price_levels compatibility mappings are not
maintained.
"""

import math
import time

import _hftlob

_MAX_UINT64 = (1 << 64) - 1


def _validate_uid(value):
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError("uid must be an integer")
    value = int(value)
    if value < 0 or value > _MAX_UINT64:
        raise ValueError("uid must be in the unsigned 64-bit range")
    return value


def _coerce_number(value, name):
    if isinstance(value, bool):
        raise TypeError(f"{name} must be numeric, not bool")
    try:
        numeric = float(value)
    except (TypeError, ValueError):
        raise TypeError(f"{name} must be numeric") from None
    if not math.isfinite(numeric):
        raise ValueError(f"{name} must be finite")
    return numeric


class _OrderListSnapshot:
    __slots__ = ("head", "tail", "count", "parent_limit")

    def __init__(self, parent_limit=None):
        self.head = None
        self.tail = None
        self.count = 0
        self.parent_limit = parent_limit

    def __len__(self):
        return self.count

    def __iter__(self):
        item = self.head
        while item is not None:
            yield item
            item = item.next_item

    def __bool__(self):
        return self.count > 0


class Order:
    """Public order metadata snapshot.

    The C order book owns all live order state. The Python object is a safe copy of
    the order data and does not retain a raw C pointer.
    """

    __slots__ = ("uid", "is_bid", "size", "price", "timestamp", "next_item", "previous_item", "root")

    def __init__(self, uid, is_bid, size, price, root=None, timestamp=None, next_item=None, previous_item=None):
        self.uid = _validate_uid(uid)
        self.is_bid = bool(is_bid)
        self.size = _coerce_number(size, "size")
        self.price = _coerce_number(price, "price")
        self.timestamp = time.time() if timestamp is None else float(timestamp)
        self.root = root
        self.next_item = next_item
        self.previous_item = previous_item

    @property
    def parent_limit(self):
        if self.root is not None:
            return getattr(self.root, "parent_limit", None)
        return None

    def __eq__(self, other):
        if not isinstance(other, Order):
            return NotImplemented
        return (
            self.uid == other.uid
            and self.is_bid == other.is_bid
            and self.size == other.size
            and self.price == other.price
            and self.timestamp == other.timestamp
        )

    def __repr__(self):
        return f"Order(uid={self.uid}, is_bid={self.is_bid}, size={self.size}, price={self.price}, timestamp={self.timestamp})"

    def __str__(self):
        return self.__repr__()


class LimitLevel:
    """Snapshot of a book price level.

    This is a copied view of the current C-side level; it never retains a pointer to the
    underlying book memory.
    """

    __slots__ = ("price", "size", "total_volume", "order_count", "orders")

    def __init__(self, price, size, total_volume=None, order_count=None, orders=None):
        self.price = float(price)
        self.size = float(size)
        self.total_volume = float(total_volume if total_volume is not None else self.size * self.price)
        self.order_count = int(order_count if order_count is not None else 0)
        self.orders = orders if orders is not None else _OrderListSnapshot(self)
        if self.orders.parent_limit is None:
            self.orders.parent_limit = self

    @property
    def volume(self):
        return self.price * self.size

    @property
    def head(self):
        return self.orders.head

    @property
    def tail(self):
        return self.orders.tail

    def __len__(self):
        return self.order_count

    def __iter__(self):
        return iter(self.orders)

    def __eq__(self, other):
        if not isinstance(other, LimitLevel):
            return NotImplemented
        return (
            self.price == other.price
            and self.size == other.size
            and self.total_volume == other.total_volume
            and self.order_count == other.order_count
        )

    def __repr__(self):
        return f"LimitLevel(price={self.price}, size={self.size}, total_volume={self.total_volume}, order_count={self.order_count})"


class LimitOrderBook:
    """Facade over the C-backed order book.

    This wrapper keeps the C book as the single source of truth and exposes Python-side
    snapshot objects instead of live AVL internals. The public view is therefore a
    compatibility façade: best_bid/best_ask, top_level, levels(), and order snapshots are
    rebuilt from the C book on demand, while bids/asks are snapshot lists rather than live
    tree roots. Legacy private internals like _orders and _price_levels are intentionally
    not exposed by the C-backed implementation.
    """

    def __init__(self):
        self._book = _hftlob.BookHandle()
        self._timestamps = {}

    def __del__(self):
        try:
            self.close()
        except Exception:
            pass

    def close(self):
        book = getattr(self, "_book", None)
        if book is None:
            return
        self._book = None
        try:
            book.close()
        except Exception:
            pass

    def _ensure_open(self):
        if self._book is None:
            raise RuntimeError("book is closed")

    def _order_from_dict(self, order_data, root=None, timestamp=None):
        if order_data is None:
            return None
        uid = int(order_data["uid"])
        if timestamp is None:
            timestamp = self._timestamps.get(uid)
        order = Order(
            uid=uid,
            is_bid=order_data["is_bid"],
            size=order_data["size"],
            price=order_data["price"],
            root=root,
            timestamp=timestamp,
        )
        return order

    def _remember_timestamp(self, uid, timestamp):
        if timestamp is None:
            self._timestamps.pop(int(uid), None)
            return
        self._timestamps[int(uid)] = float(timestamp)

    def _snapshot_order_list(self, order_dicts, parent_limit=None):
        snapshot = _OrderListSnapshot(parent_limit)
        if not order_dicts:
            return snapshot

        items = [self._order_from_dict(data, root=parent_limit) for data in order_dicts]
        for index, item in enumerate(items):
            item.previous_item = items[index - 1] if index > 0 else None
            item.next_item = items[index + 1] if index + 1 < len(items) else None
        snapshot.head = items[0]
        snapshot.tail = items[-1]
        snapshot.count = len(items)
        return snapshot

    def _to_level(self, data, is_bid):
        if data is None:
            return None
        order_dicts = self._book.get_level_orders(is_bid, data["price"]) if self._book is not None else []
        level = LimitLevel(
            price=data["price"],
            size=data["size"],
            total_volume=data["total_volume"],
            order_count=data["order_count"],
            orders=self._snapshot_order_list(order_dicts, parent_limit=None),
        )
        level.orders.parent_limit = level
        return level

    def _snapshot_levels(self, is_bid):
        self._ensure_open()
        side_data = self._book.levels(is_bid)
        levels = [self._to_level(level, is_bid) for level in side_data]
        return levels

    @property
    def best_bid(self):
        if self._book is None:
            return None
        data = self._book.best_bid()
        return self._to_level(data, True) if data is not None else None

    @property
    def best_ask(self):
        if self._book is None:
            return None
        data = self._book.best_ask()
        return self._to_level(data, False) if data is not None else None

    @property
    def bids(self):
        return self._snapshot_levels(True)

    @property
    def asks(self):
        return self._snapshot_levels(False)

    @property
    def top_level(self):
        return (self.best_bid, self.best_ask)

    def add(self, order):
        self._ensure_open()
        if not isinstance(order, Order):
            raise TypeError("order must be an Order instance")
        current = self._book.get_order(order.uid)
        if current is not None:
            raise ValueError(f"duplicate uid: {order.uid}")
        self._book.add_order(order.uid, order.is_bid, order.size, order.price)
        self._remember_timestamp(order.uid, order.timestamp)
        return order

    def remove(self, order_or_uid):
        self._ensure_open()
        if isinstance(order_or_uid, Order):
            uid = order_or_uid.uid
        else:
            uid = _validate_uid(order_or_uid)
        current = self._book.get_order(uid)
        if current is None:
            return False
        removed = self._order_from_dict(current, timestamp=self._timestamps.get(uid))
        self._book.remove_order(uid)
        self._timestamps.pop(uid, None)
        return removed

    def update(self, order):
        self._ensure_open()
        if not isinstance(order, Order):
            raise TypeError("order must be an Order instance")

        if order.size == 0:
            return self.remove(order.uid)

        current = self._book.get_order(order.uid)
        if current is None:
            raise KeyError(order.uid)

        stored_timestamp = self._timestamps.get(order.uid)
        if order.timestamp is not None:
            stored_timestamp = float(order.timestamp)
        elif stored_timestamp is not None:
            stored_timestamp = float(stored_timestamp)

        original_side = bool(current["is_bid"])
        original_price = float(current["price"])
        original_size = float(current["size"])
        original_timestamp = self._timestamps.get(order.uid)

        if original_side != bool(order.is_bid) or original_price != float(order.price):
            remove_ret = self._book.remove_order(order.uid)
            if remove_ret == 0:
                raise RuntimeError(f"failed to remove order {order.uid} before replacement")

            try:
                add_ret = self._book.add_order(order.uid, bool(order.is_bid), float(order.size), float(order.price))
                if add_ret == 0:
                    raise RuntimeError(f"failed to add replacement order {order.uid}")
            except Exception as replace_exc:
                try:
                    self._book.add_order(order.uid, original_side, original_size, original_price)
                except Exception as rollback_exc:
                    self._timestamps.pop(order.uid, None)
                    raise RuntimeError(
                        f"failed to replace order {order.uid}: replacement error={replace_exc}; rollback error={rollback_exc}"
                    ) from None
                self._remember_timestamp(order.uid, original_timestamp)
                raise RuntimeError(f"failed to replace order {order.uid}: {replace_exc}") from replace_exc
            self._remember_timestamp(order.uid, stored_timestamp)
            return order

        ret = self._book.replace_order(order.uid, float(order.size), float(order.price))
        if ret == 0:
            raise ValueError(f"failed to update order {order.uid}")
        self._remember_timestamp(order.uid, stored_timestamp)
        return order

    def process(self, order):
        self._ensure_open()
        if not isinstance(order, Order):
            raise TypeError("order must be an Order instance")
        current = self._book.get_order(order.uid)
        if current is None:
            if order.size == 0:
                return None
            self.add(order)
            return
        if order.size == 0:
            self.remove(order.uid)
            return
        self.update(order)

    def levels(self, depth=None):
        self._ensure_open()
        if depth is not None:
            if isinstance(depth, bool) or not isinstance(depth, int):
                raise TypeError("depth must be an integer or None")
            if depth < 0:
                raise ValueError("depth must be non-negative")

        bids = self._snapshot_levels(True)
        asks = self._snapshot_levels(False)
        if depth is not None:
            bids = bids[:depth]
            asks = asks[:depth]
        return {"bids": bids, "asks": asks}


__all__ = ["Order", "LimitLevel", "LimitOrderBook"]

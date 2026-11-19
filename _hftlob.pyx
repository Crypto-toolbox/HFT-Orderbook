# cython: language_level=3

from cpython.mem cimport PyMem_Free, PyMem_Malloc

cimport hftlob


cdef object _UINT64_MAX = (1 << 64) - 1


cdef inline hftlob.OrderId _validate_uid(object value, str name):
    if isinstance(value, bool):
        raise TypeError(f"{name} must be an integer, not bool")
    if not isinstance(value, int):
        raise TypeError(f"{name} must be an integer")
    value = int(value)
    if value < 0 or value > _UINT64_MAX:
        raise ValueError(f"{name} must be in the unsigned 64-bit range")
    return <hftlob.OrderId>value


cdef class BookHandle:
    cdef hftlob.Book *_book

    cdef void _require_live_book(self) except *:
        if self._book == NULL:
            raise RuntimeError("book is closed")

    def __cinit__(self):
        self._book = hftlob.createBook()
        if self._book == NULL:
            raise MemoryError("failed to allocate C order book")

    def __dealloc__(self):
        if self._book != NULL:
            hftlob.freeBook(self._book)
            self._book = NULL

    cpdef close(self):
        if self._book == NULL:
            return
        hftlob.freeBook(self._book)
        self._book = NULL

    cpdef add_order(self, object order_id, bint is_bid, double shares, double price):
        self._require_live_book()
        cdef hftlob.OrderId oid
        cdef hftlob.Order *order
        cdef unsigned side = hftlob.HFTLOB_BUY if is_bid else hftlob.HFTLOB_SELL

        oid = _validate_uid(order_id, "order_id")
        order = hftlob.addOrderToBook(self._book, oid, side, shares, price)
        if order == NULL:
            raise ValueError("invalid order parameters or duplicate order id")
        return int(oid)

    cpdef object get_order(self, object order_id):
        self._require_live_book()
        cdef hftlob.OrderId oid
        cdef hftlob.Order *order

        oid = _validate_uid(order_id, "order_id")
        order = hftlob.getOrderFromBook(self._book, oid)
        if order == NULL:
            return None
        return {
            "uid": int(order.id),
            "is_bid": bool(order.buyOrSell == hftlob.HFTLOB_BUY),
            "size": order.shares,
            "price": order.limit,
            "timestamp": order.entryTime,
        }

    cpdef bint remove_order(self, object order_id):
        self._require_live_book()
        cdef hftlob.OrderId oid

        oid = _validate_uid(order_id, "order_id")
        return bool(hftlob.removeOrderFromBook(self._book, oid))

    cpdef int replace_order(self, object order_id, double shares, double price):
        self._require_live_book()
        cdef hftlob.OrderId oid

        oid = _validate_uid(order_id, "order_id")
        return hftlob.replaceOrderInBook(self._book, oid, shares, price)

    cpdef object best_bid(self):
        self._require_live_book()
        cdef hftlob.Limit *level = hftlob.getBestBid(self._book)
        if level == NULL:
            return None
        return {
            "price": level.limitPrice,
            "size": level.size,
            "total_volume": level.totalVolume,
            "order_count": level.orderCount,
        }

    cpdef object best_ask(self):
        self._require_live_book()
        cdef hftlob.Limit *level = hftlob.getBestAsk(self._book)
        if level == NULL:
            return None
        return {
            "price": level.limitPrice,
            "size": level.size,
            "total_volume": level.totalVolume,
            "order_count": level.orderCount,
        }

    cpdef levels(self, bint is_bid, size_t max_levels=64):
        self._require_live_book()
        cdef hftlob.LevelSnapshot *snap
        cdef size_t count
        cdef size_t requested
        cdef size_t i
        cdef unsigned side = hftlob.HFTLOB_BUY if is_bid else hftlob.HFTLOB_SELL
        cdef list result = []

        if max_levels == 0:
            return result

        count = hftlob.getLevelsSnapshot(self._book, side, NULL, 0)
        if count == 0:
            return result
        requested = count if count < max_levels else max_levels
        if requested > ((<size_t>-1) // sizeof(hftlob.LevelSnapshot)):
            raise MemoryError("level snapshot count exceeds allocation size")

        snap = <hftlob.LevelSnapshot *>PyMem_Malloc(sizeof(hftlob.LevelSnapshot) * requested)
        if snap == NULL:
            raise MemoryError("failed to allocate level snapshot buffer")

        try:
            count = hftlob.getLevelsSnapshot(self._book, side, snap, requested)
            for i in range(count):
                result.append({
                    "price": snap[i].price,
                    "size": snap[i].size,
                    "total_volume": snap[i].totalVolume,
                    "order_count": snap[i].orderCount,
                })
            return result
        finally:
            PyMem_Free(snap)

    cpdef object get_level_orders(self, bint is_bid, double price):
        self._require_live_book()
        cdef hftlob.OrderSnapshot *snap = NULL
        cdef unsigned side = hftlob.HFTLOB_BUY if is_bid else hftlob.HFTLOB_SELL
        cdef size_t count
        cdef size_t i
        cdef list result = []

        count = hftlob.getLevelOrderSnapshots(self._book, side, price, NULL, 0)
        if count == 0:
            return result
        if count > ((<size_t>-1) // sizeof(hftlob.OrderSnapshot)):
            raise MemoryError("order snapshot count exceeds allocation size")

        snap = <hftlob.OrderSnapshot *>PyMem_Malloc(sizeof(hftlob.OrderSnapshot) * count)
        if snap == NULL:
            raise MemoryError("failed to allocate order snapshot buffer")

        try:
            count = hftlob.getLevelOrderSnapshots(self._book, side, price, snap, count)
            for i in range(count):
                result.append({
                    "uid": int(snap[i].uid),
                    "is_bid": bool(snap[i].buyOrSell == hftlob.HFTLOB_BUY),
                    "size": snap[i].size,
                    "price": snap[i].price,
                    "timestamp": snap[i].timestamp,
                })
            return result
        finally:
            PyMem_Free(snap)

    cpdef object level_orders(self, bint is_bid, double price):
        return self.get_level_orders(is_bid, price)

    cdef hftlob.Book *book_ptr(self):
        self._require_live_book()
        return self._book

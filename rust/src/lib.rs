//! A limit order book after WK Selph's "How to build a fast limit order book".
//!
//! Spike: same operations as the C `Book`, written the Rust way.
//!
//! - Prices are integer ticks and quantities integer lots: no float drift.
//! - Orders live in a slab (`Vec` + free list) and link to each other by
//!   `u32` index, so there is no per-order allocation and no `Rc<RefCell>`.
//! - Price levels live in a second slab. Each order knows its level's slot,
//!   so resize/cancel never search; a `BTreeMap<Price, slot>` per side keeps
//!   prices sorted and is only touched when a level appears or empties.
//! - Order ids map to slab slots through a `HashMap` with a cheap hasher.
//!
//! ```
//! use hftlob::{Book, Side};
//!
//! let mut book = Book::new();
//! book.add(1, Side::Bid, 5, 100).unwrap();
//! book.add(2, Side::Ask, 3, 101).unwrap();
//! assert_eq!(book.best_bid(), Some((100, 5)));
//!
//! // An incoming sell for 4, no lower than 100: fills 4 against order 1.
//! let filled = book.execute(Side::Ask, 4, Some(100), |_fill| {});
//! assert_eq!(filled, 4);
//! assert_eq!(book.order(1).unwrap().qty, 1);
//! ```

use std::collections::{BTreeMap, HashMap};
use std::hash::{BuildHasherDefault, Hasher};

pub type OrderId = u64;
/// Price in integer ticks.
pub type Price = i64;
/// Quantity in integer lots.
pub type Qty = u64;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Side {
    Bid,
    Ask,
}

impl Side {
    pub fn opposite(self) -> Side {
        match self {
            Side::Bid => Side::Ask,
            Side::Ask => Side::Bid,
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Order {
    pub id: OrderId,
    pub side: Side,
    pub qty: Qty,
    pub price: Price,
}

/// One fill reported by [`Book::execute`]: `qty` traded against resting
/// order `resting_id` at `price`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Fill {
    pub resting_id: OrderId,
    pub qty: Qty,
    pub price: Price,
    /// True if the resting order was fully filled and left the book.
    pub done: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error {
    DuplicateId,
    UnknownId,
    ZeroQty,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Error::DuplicateId => "order id already in the book",
            Error::UnknownId => "order id not in the book",
            Error::ZeroQty => "quantity must be positive",
        })
    }
}

impl std::error::Error for Error {}

/// Aggregate view of one price level.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct LevelInfo {
    pub price: Price,
    pub qty: Qty,
    pub orders: u32,
}

const NIL: u32 = u32::MAX;

/// A slab slot: an order plus its links in the level's FIFO queue.
#[derive(Clone, Copy)]
struct Node {
    order: Order,
    level: u32, // slot of the order's Level
    prev: u32,  // towards the oldest order
    next: u32,  // towards the newest order
}

#[derive(Clone, Copy)]
struct Level {
    qty: Qty,
    count: u32,
    oldest: u32,
    newest: u32,
}

/// A slab: `Vec` plus a free list of reusable slots.
struct Slab<T> {
    items: Vec<T>,
    free: Vec<u32>,
}

impl<T> Default for Slab<T> {
    fn default() -> Self {
        Slab { items: Vec::new(), free: Vec::new() }
    }
}

impl<T> Slab<T> {
    fn insert(&mut self, item: T) -> u32 {
        match self.free.pop() {
            Some(slot) => {
                self.items[slot as usize] = item;
                slot
            }
            None => {
                let slot = u32::try_from(self.items.len()).expect("slab full");
                assert!(slot != NIL, "slab full");
                self.items.push(item);
                slot
            }
        }
    }

    fn release(&mut self, slot: u32) {
        self.free.push(slot);
    }
}

impl<T> std::ops::Index<u32> for Slab<T> {
    type Output = T;
    fn index(&self, slot: u32) -> &T {
        &self.items[slot as usize]
    }
}

impl<T> std::ops::IndexMut<u32> for Slab<T> {
    fn index_mut(&mut self, slot: u32) -> &mut T {
        &mut self.items[slot as usize]
    }
}

/// Cheap hasher for integer order ids (std's SipHash is DoS-resistant but
/// slow). Multiply, then fold the well-mixed high half into the low half:
/// `HashMap` picks buckets from the low bits, and a bare multiply would
/// leave ids that differ only in their high bits (e.g. `seq << 20`) all in
/// the same buckets.
#[derive(Default)]
struct IdHasher(u64);

impl Hasher for IdHasher {
    fn finish(&self) -> u64 {
        self.0
    }
    fn write(&mut self, _bytes: &[u8]) {
        unreachable!("IdHasher only hashes u64 keys")
    }
    fn write_u64(&mut self, n: u64) {
        let x = n.wrapping_mul(0x9E37_79B9_7F4A_7C15);
        self.0 = x ^ (x >> 32);
    }
}

type IdMap<V> = HashMap<OrderId, V, BuildHasherDefault<IdHasher>>;

#[derive(Default)]
pub struct Book {
    bids: BTreeMap<Price, u32>, // price -> level slot
    asks: BTreeMap<Price, u32>,
    ids: IdMap<u32>,            // order id -> node slot
    nodes: Slab<Node>,
    levels: Slab<Level>,
}

impl Book {
    pub fn new() -> Self {
        Self::default()
    }

    /// Pre-size for roughly `orders` live orders.
    pub fn with_capacity(orders: usize) -> Self {
        let mut ids = IdMap::default();
        ids.reserve(orders);
        let mut book = Book { ids, ..Self::default() };
        book.nodes.items.reserve(orders);
        book
    }

    pub fn len(&self) -> usize {
        self.ids.len()
    }

    pub fn is_empty(&self) -> bool {
        self.ids.is_empty()
    }

    pub fn order(&self, id: OrderId) -> Option<&Order> {
        self.ids.get(&id).map(|&slot| &self.nodes[slot].order)
    }

    /// Best bid as `(price, total qty)`.
    pub fn best_bid(&self) -> Option<(Price, Qty)> {
        self.bids.iter().next_back().map(|(&p, &l)| (p, self.levels[l].qty))
    }

    /// Best ask as `(price, total qty)`.
    pub fn best_ask(&self) -> Option<(Price, Qty)> {
        self.asks.iter().next().map(|(&p, &l)| (p, self.levels[l].qty))
    }

    pub fn level(&self, side: Side, price: Price) -> Option<LevelInfo> {
        self.prices(side).get(&price).map(|&l| self.info(price, l))
    }

    /// Up to `depth` levels per side, best first.
    pub fn depth(&self, depth: usize) -> (Vec<LevelInfo>, Vec<LevelInfo>) {
        let info = |(&price, &l): (&Price, &u32)| self.info(price, l);
        (
            self.bids.iter().rev().take(depth).map(info).collect(),
            self.asks.iter().take(depth).map(info).collect(),
        )
    }

    /// Add a resting order at the back of its price level's queue.
    pub fn add(&mut self, id: OrderId, side: Side, qty: Qty, price: Price) -> Result<(), Error> {
        if qty == 0 {
            return Err(Error::ZeroQty);
        }
        let slot = self.nodes.insert(Node {
            order: Order { id, side, qty, price },
            level: NIL,
            prev: NIL,
            next: NIL,
        });
        if self.ids.try_insert_slot(id, slot).is_err() {
            self.nodes.release(slot);
            return Err(Error::DuplicateId);
        }
        self.link(slot);
        Ok(())
    }

    /// Cancel an order and return it.
    pub fn cancel(&mut self, id: OrderId) -> Result<Order, Error> {
        let slot = self.ids.remove(&id).ok_or(Error::UnknownId)?;
        self.unlink(slot);
        self.nodes.release(slot);
        Ok(self.nodes[slot].order)
    }

    /// Change an order's quantity and/or price.
    ///
    /// A quantity-only change keeps the order's place in the queue; a price
    /// change moves it to the back of its new level. Zero quantity cancels.
    pub fn replace(&mut self, id: OrderId, qty: Qty, price: Price) -> Result<(), Error> {
        let slot = *self.ids.get(&id).ok_or(Error::UnknownId)?;
        if qty == 0 {
            return self.cancel(id).map(|_| ());
        }
        let node = &mut self.nodes[slot];
        if node.order.price == price {
            let old = node.order.qty;
            node.order.qty = qty;
            let level = &mut self.levels[node.level];
            level.qty = level.qty - old + qty;
        } else {
            self.unlink(slot);
            let node = &mut self.nodes[slot];
            node.order.qty = qty;
            node.order.price = price;
            self.link(slot);
        }
        Ok(())
    }

    /// Execute an incoming order of `qty` on side `aggressor` against the
    /// other side, in price-time priority, up to `limit` (`None` = market).
    ///
    /// Calls `on_fill` for every fill and returns the quantity filled. The
    /// unfilled remainder is not rested (immediate-or-cancel).
    pub fn execute(
        &mut self,
        aggressor: Side,
        qty: Qty,
        limit: Option<Price>,
        mut on_fill: impl FnMut(Fill),
    ) -> Qty {
        let mut remaining = qty;
        while remaining > 0 {
            let resting_side = aggressor.opposite();
            let best = match resting_side {
                Side::Ask => self.asks.first_key_value(),
                Side::Bid => self.bids.last_key_value(),
            };
            let Some((&price, &level_slot)) = best else { break };
            let crosses = match (aggressor, limit) {
                (_, None) => true,
                (Side::Bid, Some(l)) => price <= l,
                (Side::Ask, Some(l)) => price >= l,
            };
            if !crosses {
                break;
            }

            let slot = self.levels[level_slot].oldest;
            let node = &mut self.nodes[slot];
            let fill = node.order.qty.min(remaining);
            let done = fill == node.order.qty;
            on_fill(Fill {
                resting_id: node.order.id,
                qty: fill,
                price,
                done,
            });
            remaining -= fill;

            if done {
                let id = node.order.id;
                self.ids.remove(&id);
                self.unlink(slot);
                self.nodes.release(slot);
            } else {
                node.order.qty -= fill;
                self.levels[level_slot].qty -= fill;
            }
        }
        qty - remaining
    }

    fn prices(&self, side: Side) -> &BTreeMap<Price, u32> {
        match side {
            Side::Bid => &self.bids,
            Side::Ask => &self.asks,
        }
    }

    fn prices_mut(&mut self, side: Side) -> &mut BTreeMap<Price, u32> {
        match side {
            Side::Bid => &mut self.bids,
            Side::Ask => &mut self.asks,
        }
    }

    fn info(&self, price: Price, level: u32) -> LevelInfo {
        let l = &self.levels[level];
        LevelInfo { price, qty: l.qty, orders: l.count }
    }

    /// Append the node at `slot` to the back (newest end) of its level,
    /// creating the level if needed.
    fn link(&mut self, slot: u32) {
        let Order { side, qty, price, .. } = self.nodes[slot].order;
        let level_slot = match self.prices(side).get(&price) {
            Some(&l) => l,
            None => {
                let l = self.levels.insert(Level { qty: 0, count: 0, oldest: NIL, newest: NIL });
                self.prices_mut(side).insert(price, l);
                l
            }
        };
        let level = &mut self.levels[level_slot];
        let prev = level.newest;
        level.newest = slot;
        if prev == NIL {
            level.oldest = slot;
        }
        level.qty += qty;
        level.count += 1;
        if prev != NIL {
            self.nodes[prev].next = slot;
        }
        let node = &mut self.nodes[slot];
        node.level = level_slot;
        node.prev = prev;
        node.next = NIL;
    }

    /// Detach the node at `slot` from its level, dropping emptied levels.
    fn unlink(&mut self, slot: u32) {
        let Node { order, level: level_slot, prev, next } = self.nodes[slot];
        if prev != NIL {
            self.nodes[prev].next = next;
        }
        if next != NIL {
            self.nodes[next].prev = prev;
        }
        let level = &mut self.levels[level_slot];
        if prev == NIL {
            level.oldest = next;
        }
        if next == NIL {
            level.newest = prev;
        }
        level.qty -= order.qty;
        level.count -= 1;
        if level.count == 0 {
            self.prices_mut(order.side).remove(&order.price);
            self.levels.release(level_slot);
        }
    }

    /// Orders at a level, oldest first. Mainly for tests and debugging.
    pub fn queue(&self, side: Side, price: Price) -> Vec<OrderId> {
        let mut out = Vec::new();
        if let Some(&level) = self.prices(side).get(&price) {
            let mut slot = self.levels[level].oldest;
            while slot != NIL {
                let node = &self.nodes[slot];
                out.push(node.order.id);
                slot = node.next;
            }
        }
        out
    }
}

/// `HashMap::try_insert` is unstable; this is the stable equivalent.
trait TryInsertSlot {
    fn try_insert_slot(&mut self, id: OrderId, slot: u32) -> Result<(), ()>;
}

impl TryInsertSlot for IdMap<u32> {
    fn try_insert_slot(&mut self, id: OrderId, slot: u32) -> Result<(), ()> {
        use std::collections::hash_map::Entry;
        match self.entry(id) {
            Entry::Occupied(_) => Err(()),
            Entry::Vacant(v) => {
                v.insert(slot);
                Ok(())
            }
        }
    }
}

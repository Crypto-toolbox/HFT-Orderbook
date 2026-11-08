use hftlob::{Book, Error, Fill, Order, Side};

#[test]
fn add_and_best_prices() {
    let mut book = Book::new();
    assert_eq!(book.best_bid(), None);
    book.add(1, Side::Bid, 5, 100).unwrap();
    book.add(2, Side::Bid, 3, 101).unwrap();
    book.add(3, Side::Bid, 2, 101).unwrap();
    book.add(4, Side::Ask, 4, 103).unwrap();
    book.add(5, Side::Ask, 1, 102).unwrap();
    assert_eq!(book.best_bid(), Some((101, 5)));
    assert_eq!(book.best_ask(), Some((102, 1)));
    assert_eq!(book.queue(Side::Bid, 101), vec![2, 3]);
    assert_eq!(book.level(Side::Bid, 101).unwrap().orders, 2);
    assert_eq!(book.len(), 5);

    let (bids, asks) = book.depth(5);
    assert_eq!(
        bids.iter().map(|l| l.price).collect::<Vec<_>>(),
        vec![101, 100]
    );
    assert_eq!(
        asks.iter().map(|l| l.price).collect::<Vec<_>>(),
        vec![102, 103]
    );
}

#[test]
fn rejects_invalid_orders() {
    let mut book = Book::new();
    book.add(1, Side::Bid, 1, 100).unwrap();
    assert_eq!(book.add(1, Side::Ask, 1, 105), Err(Error::DuplicateId));
    assert_eq!(book.add(2, Side::Bid, 0, 100), Err(Error::ZeroQty));
    assert_eq!(book.cancel(9), Err(Error::UnknownId));
    assert_eq!(book.replace(9, 1, 1), Err(Error::UnknownId));
    assert_eq!(book.len(), 1);
    // The rejected duplicate must not have leaked into the book.
    assert_eq!(book.queue(Side::Ask, 105), Vec::<u64>::new());
    book.add(3, Side::Bid, 1, 100).unwrap();
    assert_eq!(book.queue(Side::Bid, 100), vec![1, 3]);
}

#[test]
fn cancel_falls_back_to_next_best() {
    let mut book = Book::new();
    book.add(1, Side::Bid, 5, 100).unwrap();
    book.add(2, Side::Bid, 3, 101).unwrap();
    book.add(3, Side::Bid, 2, 101).unwrap();
    assert_eq!(book.cancel(2).unwrap().qty, 3);
    assert_eq!(book.best_bid(), Some((101, 2)));
    book.cancel(3).unwrap();
    assert_eq!(book.best_bid(), Some((100, 5)));
    assert_eq!(book.level(Side::Bid, 101), None);
    book.cancel(1).unwrap();
    assert!(book.is_empty());
    assert_eq!(book.best_bid(), None);
}

#[test]
fn replace_keeps_or_loses_priority() {
    let mut book = Book::new();
    book.add(1, Side::Ask, 5, 100).unwrap();
    book.add(2, Side::Ask, 3, 100).unwrap();

    book.replace(1, 2, 100).unwrap(); // size only: stays first
    assert_eq!(book.queue(Side::Ask, 100), vec![1, 2]);
    assert_eq!(book.best_ask(), Some((100, 5)));

    book.add(3, Side::Ask, 1, 99).unwrap();
    book.replace(1, 2, 99).unwrap(); // new price: back of the queue
    assert_eq!(book.queue(Side::Ask, 99), vec![3, 1]);
    assert_eq!(book.best_ask(), Some((99, 3)));

    book.replace(2, 3, 98).unwrap(); // moving the last order drops the level
    assert_eq!(book.level(Side::Ask, 100), None);
    book.replace(2, 0, 98).unwrap(); // zero cancels
    assert!(book.order(2).is_none());
}

#[test]
fn execute_buy_in_price_time_priority() {
    let mut book = Book::new();
    book.add(1, Side::Ask, 5, 101).unwrap();
    book.add(2, Side::Ask, 3, 101).unwrap();
    book.add(3, Side::Ask, 4, 102).unwrap();
    book.add(4, Side::Ask, 9, 105).unwrap();
    book.add(5, Side::Bid, 9, 100).unwrap();

    let mut fills = Vec::new();
    let filled = book.execute(Side::Bid, 10, Some(102), |f| fills.push(f));
    assert_eq!(filled, 10);
    assert_eq!(
        fills,
        vec![
            Fill {
                resting_id: 1,
                qty: 5,
                price: 101,
                done: true
            },
            Fill {
                resting_id: 2,
                qty: 3,
                price: 101,
                done: true
            },
            Fill {
                resting_id: 3,
                qty: 2,
                price: 102,
                done: false
            },
        ]
    );
    assert_eq!(book.best_ask(), Some((102, 2)));
    assert_eq!(book.best_bid(), Some((100, 9)));

    assert_eq!(book.execute(Side::Bid, 50, Some(101), |_| {}), 0);
    assert_eq!(book.execute(Side::Bid, 50, None, |_| {}), 11);
    assert_eq!(book.best_ask(), None);
    assert_eq!(book.len(), 1);
}

#[test]
fn execute_sell() {
    let mut book = Book::new();
    book.add(1, Side::Bid, 5, 99).unwrap();
    book.add(2, Side::Bid, 5, 100).unwrap();
    book.add(3, Side::Bid, 5, 98).unwrap();
    assert_eq!(book.execute(Side::Ask, 7, Some(99), |_| {}), 7);
    assert_eq!(book.best_bid(), Some((99, 3)));
    assert_eq!(book.execute(Side::Ask, 10, Some(99), |_| {}), 3);
    assert_eq!(book.execute(Side::Ask, 10, None, |_| {}), 5);
    assert!(book.is_empty());
}

// --- Randomised test against a naive reference model ------------------

/// Deliberately dumb: orders in a Vec in queue order.
#[derive(Default)]
struct Reference {
    orders: Vec<Order>,
}

impl Reference {
    fn best(&self, side: Side) -> Option<i64> {
        let prices = self
            .orders
            .iter()
            .filter(|o| o.side == side)
            .map(|o| o.price);
        match side {
            Side::Bid => prices.max(),
            Side::Ask => prices.min(),
        }
    }
    fn level_qty(&self, side: Side, price: i64) -> u64 {
        self.orders
            .iter()
            .filter(|o| o.side == side && o.price == price)
            .map(|o| o.qty)
            .sum()
    }
    fn queue(&self, side: Side, price: i64) -> Vec<u64> {
        self.orders
            .iter()
            .filter(|o| o.side == side && o.price == price)
            .map(|o| o.id)
            .collect()
    }
    fn execute(&mut self, aggressor: Side, mut qty: u64, limit: Option<i64>) -> Vec<Fill> {
        let mut fills = Vec::new();
        while qty > 0 {
            let side = aggressor.opposite();
            let Some(price) = self.best(side) else { break };
            let crosses = match (aggressor, limit) {
                (_, None) => true,
                (Side::Bid, Some(l)) => price <= l,
                (Side::Ask, Some(l)) => price >= l,
            };
            if !crosses {
                break;
            }
            let i = self
                .orders
                .iter()
                .position(|o| o.side == side && o.price == price)
                .unwrap();
            let fill = self.orders[i].qty.min(qty);
            let done = fill == self.orders[i].qty;
            fills.push(Fill {
                resting_id: self.orders[i].id,
                qty: fill,
                price,
                done,
            });
            qty -= fill;
            if done {
                self.orders.remove(i);
            } else {
                self.orders[i].qty -= fill;
            }
        }
        fills
    }
}

/// xorshift64*: tiny, deterministic, no dependencies.
struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u64 {
        self.0 ^= self.0 >> 12;
        self.0 ^= self.0 << 25;
        self.0 ^= self.0 >> 27;
        self.0.wrapping_mul(0x2545_F491_4F6C_DD1D)
    }
    fn below(&mut self, n: u64) -> u64 {
        self.next() % n
    }
}

#[test]
fn random_order_flow_matches_reference() {
    let mut rng = Rng(0x5EED);
    let mut book = Book::new();
    let mut reference = Reference::default();

    for op in 0..20_000u64 {
        let roll = rng.below(100);
        let live = reference.orders.len() as u64;
        // Overlapping bid/ask price ranges, so books cross and executes fill.
        let price = |side: Side, rng: &mut Rng| match side {
            Side::Bid => 1 + rng.below(40) as i64,
            Side::Ask => 21 + rng.below(40) as i64,
        };
        if live > 0 && roll < 10 {
            let aggressor = if rng.below(2) == 0 {
                Side::Bid
            } else {
                Side::Ask
            };
            let qty = 1 + rng.below(30);
            let limit = if rng.below(4) == 0 {
                None
            } else {
                Some(rng.below(70) as i64)
            };
            let mut fills = Vec::new();
            let filled = book.execute(aggressor, qty, limit, |f| fills.push(f));
            let expected = reference.execute(aggressor, qty, limit);
            assert_eq!(fills, expected, "op {op}: execute fills");
            assert_eq!(filled, expected.iter().map(|f| f.qty).sum::<u64>());
        } else if live > 0 && roll < 25 {
            let i = rng.below(live) as usize;
            let Order { id, side, .. } = reference.orders[i];
            let qty = rng.below(10); // 0 cancels
            let new_price = if rng.below(2) == 0 {
                reference.orders[i].price
            } else {
                price(side, &mut rng)
            };
            book.replace(id, qty, new_price).unwrap();
            if qty == 0 {
                reference.orders.remove(i);
            } else if new_price == reference.orders[i].price {
                reference.orders[i].qty = qty;
            } else {
                let mut moved = reference.orders.remove(i);
                moved.qty = qty;
                moved.price = new_price;
                reference.orders.push(moved);
            }
        } else if live > 0 && roll < 60 {
            let i = rng.below(live) as usize;
            let id = reference.orders[i].id;
            assert_eq!(book.cancel(id).unwrap(), reference.orders.remove(i));
        } else {
            let side = if rng.below(2) == 0 {
                Side::Bid
            } else {
                Side::Ask
            };
            let order = Order {
                id: op,
                side,
                qty: 1 + rng.below(10),
                price: price(side, &mut rng),
            };
            book.add(order.id, order.side, order.qty, order.price)
                .unwrap();
            reference.orders.push(order);
        }

        // Compare full state.
        assert_eq!(book.len(), reference.orders.len(), "op {op}: len");
        for side in [Side::Bid, Side::Ask] {
            let best = reference
                .best(side)
                .map(|p| (p, reference.level_qty(side, p)));
            let got = if side == Side::Bid {
                book.best_bid()
            } else {
                book.best_ask()
            };
            assert_eq!(got, best, "op {op}: best {side:?}");
        }
        if op % 100 == 0 {
            for o in &reference.orders {
                assert_eq!(book.order(o.id), Some(o), "op {op}: order {}", o.id);
                assert_eq!(
                    book.queue(o.side, o.price),
                    reference.queue(o.side, o.price)
                );
            }
        }
    }
}

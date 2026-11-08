# hftlob (Rust spike)

A Rust take on the same limit order book as the C `Book` in `../src`, built to compare the two.

**Status: spike.** Tested and benchmarked, but not packaged, published or wired into CI.

## How it's built

Same idea as the C version, done the Rust way:

- **Integer prices and quantities:** prices are `i64` ticks and quantities are `u64` lots, so there's no float drift.
- **Slabs instead of pointers:** orders and price levels live in `Vec` slabs with free lists, linked by `u32` indices. No `Rc<RefCell>`, no `unsafe`, and no allocation per order.
- **Each order knows its level:** resize and cancel never search, just like `parentLimit` in C.
- **`BTreeMap<Price, level>` per side** keeps prices sorted, instead of a hand-written AVL tree. It's only touched when a level appears or empties.
- **No dependencies.**

```rust
use hftlob::{Book, Side};

let mut book = Book::new();
book.add(1, Side::Bid, 5, 100)?;
book.add(2, Side::Ask, 3, 101)?;
book.replace(1, 2, 100)?;          // new size: keeps queue spot
book.cancel(2)?;
let filled = book.execute(Side::Ask, 4, Some(100), |fill| println!("{fill:?}"));
```

## Try it

```bash
cargo test                                         # unit tests + randomised reference test
cargo run --release --example bench -- [orders] [levels]
```

The randomised test runs 20,000 adds, cancels, replaces and executes against a deliberately naive model (a plain `Vec` of orders). It compares every fill and the best prices after every step.

## Rust vs C

1M orders over 5,000 price levels per side, best of 3 runs, same workload as the C benchmark (`bench/bench.c`, added in #36), on the same cloud VM:

| Operation | C (pools + flat maps) | Rust spike |
|---|---|---|
| add | ~170 ns | ~135 ns |
| resize | ~40 ns | ~50 ns |
| cancel | ~70 ns | ~65 ns |
| execute (per call) | ~270 ns | ~160 ns |
| teardown | ~3 ms | ~3 ms |

Rust's execute is faster mainly because a B-tree finds the next best price after a level empties with far fewer cache misses than the C AVL tree.

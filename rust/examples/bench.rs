//! Same workload as the C `hftlob_bench` (bench/bench.c, added in #36).
//!
//!     cargo run --release --example bench -- [orders] [levels]

use hftlob::{Book, Side};
use std::time::Instant;

const RUNS: usize = 3;

fn main() {
    let mut args = std::env::args().skip(1);
    let n: u64 = args.next().map_or(1_000_000, |a| a.parse().unwrap());
    let levels: u64 = args.next().map_or(5000, |a| a.parse().unwrap());

    // Same shape as the C bench; a tiny LCG instead of rand().
    let mut seed = 1u64;
    let mut rand = move || {
        seed = seed
            .wrapping_mul(6364136223846793005)
            .wrapping_add(1442695040888963407);
        seed >> 33
    };
    let orders: Vec<(Side, u64, i64)> = (0..n)
        .map(|i| {
            let side = if i & 1 == 1 { Side::Bid } else { Side::Ask };
            let qty = 1 + rand() % 100;
            let offset = if side == Side::Bid { 0 } else { levels };
            (side, qty, (rand() % levels + offset) as i64)
        })
        .collect();

    let mut best = [f64::MAX; 5];
    let mut calls = 0u64;
    for _ in 0..RUNS {
        let mut book = Book::new();

        let t = Instant::now();
        for (id, &(side, qty, price)) in orders.iter().enumerate() {
            book.add(id as u64, side, qty, price).unwrap();
        }
        best[0] = best[0].min(t.elapsed().as_nanos() as f64 / n as f64);

        let t = Instant::now();
        for id in (0..n).step_by(2) {
            let (_, qty, price) = orders[id as usize];
            book.replace(id, qty + 1, price).unwrap();
        }
        best[1] = best[1].min(t.elapsed().as_nanos() as f64 / (n / 2) as f64);

        let t = Instant::now();
        for id in (0..n).step_by(4) {
            book.cancel(id).unwrap();
            book.cancel(id + 1).unwrap();
        }
        best[2] = best[2].min(t.elapsed().as_nanos() as f64 / (n / 2) as f64);

        let t = Instant::now();
        calls = 0;
        while book.best_bid().is_some() {
            book.execute(Side::Ask, 50, None, |_| {});
            calls += 1;
        }
        best[3] = best[3].min(t.elapsed().as_nanos() as f64 / calls as f64);

        let t = Instant::now();
        drop(book);
        best[4] = best[4].min(t.elapsed().as_secs_f64() * 1e3);
    }

    println!("{n} orders, {levels} levels per side, best of {RUNS} runs");
    println!("  add      {:7.1} ns/op", best[0]);
    println!("  resize   {:7.1} ns/op", best[1]);
    println!("  cancel   {:7.1} ns/op", best[2]);
    println!("  execute  {:7.1} ns/call ({calls} calls)", best[3]);
    println!("  drop     {:7.1} ms", best[4]);
}

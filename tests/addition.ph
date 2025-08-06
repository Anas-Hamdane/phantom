fn main() -> i32 {
  let x: i32 = 5;
  let y: i64 = 10;
  let z: f64 = 20.1;

  let result: i32 = (x + y) + (x + z) + (y + z) + (z + 2.3);
  // 15 + 25.1 + 30.1 + 22.4
  return result;
}

fn main() -> i32 {
  let x: i32 = 10;
  let y: i64 = 20;

  let a: f32 = 15.51;
  let b: f64 = 25.615;

  let result: i32 = (x - 5) - (y - 5) - (y - x) - (x - y) + (b - a) - (a - b) + 100;
  // result should be 110
  return result;
}

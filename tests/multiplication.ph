fn main() -> i32 {
  let a: i32 = 2;
  let b: i64 = 3;

  let c: f32 = 4.6;
  let d: f64 = 5.6;

  let result: i32 = a * b * c * d;
  // 2 * 3 * 4.6 * 5.6 = 154.56, should return 155
  return result;
}

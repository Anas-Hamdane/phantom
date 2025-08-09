fn main() -> i32 {
  let x : f32 = 1.2;
  let y : f64 = x;

  let a : f64 = 2.4;
  let b : f32 = a;

  let c: i8 = y;
  let d: i16 = y;
  let e: i32 = y;
  let f: i64 = y;

  let g: f32 = c;
  g = d;
  g = e;
  g = f;

  let h: f64 = c;
  h = d;
  h = e;
  h = f;
  return 0;
}

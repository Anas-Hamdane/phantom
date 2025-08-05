fn main() -> i32 {
  let a: i32 = 1;

  // 1-constant int to float
  let b: f32 = 1;

  // 2-virtual reg int to float
  let c: f32 = a;

  // 3-binop int to float
  let d: f32 = a + 1;

  // 4-constant int to double
  let e: f64 = 1;

  // 5-virtual reg int to double
  let f: f64 = a;

  // 6-binop int to float
  let g: f64 = a + 1;

  //////////////////////////////////////////////

  let h: f32 = 1.2;

  // 1-virtual reg float to int
  let i: i32 = h;

  // 2-binop float to int
  let j: f32 = h + 2.4;

  // 4-virtual reg float to double
  let k: f64 = h;

  // 5-binop float to double
  let l: f64 = h + 1.2;

  ////////////////////////////////////////////////

  let m: f64 = 2.4;

  // 1-virtual reg double to int
  let u: i32 = m;

  // 2-binop double to int
  let v: f32 = m + 2.4;

  // 4-virtual reg double to float
  let w: f32 = m;

  // 5-binop double to float
  let x: f32 = m + 1.2;
  return 0;
}

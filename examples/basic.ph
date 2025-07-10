fn main() => int {
  let x: short = 9;
  let y: int = 2;

  let z = y + x;
  let a = &z;

  *a = 20;
  let b = &a;
  **b = 30;
  return **b;
}

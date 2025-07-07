fn main(argc: int) => int {
  let a = 10;
  let b = 5;

  let sum = a + b;
  let diff = a - b;
  let prod = a * b;
  let quot = a / b;

  let addr = &sum;
  let deref = *addr;

  let result = sum + diff + prod + quot + deref;
  return result;
}

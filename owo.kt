fun fib(n) {
  if (n <= 1)
    return n;
  return fib(n - 2) + fib(n - 1);
}

var result = fib(2);

print(result);

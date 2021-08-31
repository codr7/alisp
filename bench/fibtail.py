from bench import bench

print(bench(10000, '''
def fib(n, a, b):
  return a if n == 0 else b if n == 1 else fib(n-1, b, a+b)
''', '''
  fib(70, 0, 1)
'''))

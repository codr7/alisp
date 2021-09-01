## aLisp

### intro
aLisp aims to become a hackable, embeddable, reasonably fast interpreted custom Lisp implemented in portable C. The current version weighs in at 4 kloc and supports all features described in this document.

### quirks
- Parens are used for calls, brackets for lists.
- Infix calls are supported with `.` between first argument and call.
- Lists are passed by reference.

### setup
aLisp requires CMake and a C compiler to build.

```
$ cd alisp
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./alisp
Welcome to aLisp v6

Return on empty line evaluates,
(reset) clears the stack and Ctrl+D exits.

  (func fibrec [n:Int] [Int]
    (if n.(< 2) n n.(- 1).(fibrec).(+ n.(- 2).(fibrec))))
  10.(fibrec)
[55]
```

### the stack
aLisp exposes the stack to a greater extent than usual among Lisps.

Values are pushed on the stack.

```
  1 2 3
  
[1 2 3]
```

`_` may be specified where values are expected to pop the stack.

```
  42 (if _ 1 2)
  
[1]
```

`d` may be used to drop values.

```
  1 2 3 (d)
  
[1 2]

  (d 2)
  
[]
```

`d` may alternatively be specified as a call flag to drop all returned values.

```
  35.(+ 7)
  
[42]
```
```
  35.(+:d 7)
  
[]
```

`dup` may be used to repeat the top value.

```
  1 2 3 (dup)

[1 2 3 3]
```

`swap` may be used to swap the top two values.

```
  1 2 3 (swap)

[1 3 2]
```

### bindings
Values may be bound to identifiers using `let`, literals are automagically bound at compile time.

```
  (let [x 35 y 7]
    x.(+ y))
    
[42]
```

Bindings may be aliased at compile time using `alias`.

```
  Int.(alias foo)
  foo
  
[Int]
```

### equality
`is` returns `T` if both arguments are the same value.

```
  [].is([])
  
[F]
```

`=` returns `T` if both arguments are equal.

```
  [].=([])
  
[T]
```

### order

`<` and `>` may be used to order values.

```
  [1 2 3].<([1 2 4])
  
[T]
```

### booleans
The boolean type has two values, `T` and `F`.

```
  (if F 1 2)
  
[2]
```

Every value has a boolean representation; most are true but zero, empty lists etc. are false.

```
  (if [] 1 2)
  
[2]
```

### pairs
Pairs may be formed using `:`.

```
  1:2

[1:2]
```

### lists
List literals may be specified by enclosing code in brackets.

```
  [1 2 3]
  
[[1 2 3]]
```

### functions
New functions may be defined using `func`.

```
  (func foo [] [Int] 42)
  (foo)
  
[42]
```

Functions capture their defining environment.

```
  (let [bar 42]
    (func foo [] [Int] bar))
  (foo)
  
[42]
```

Anonymous arguments are left on the stack.

```
  (func foo [Int] [Int] _.(+ 7))
  (foo 35)

[42]
```

Specifying names binds the values.

```
  (func foo [n:Int] [Int] n.(+ 7))
  (foo 35)

[42]
```

### types
The following types are provided out of the box, adding more is trivial.

- Any - Any value
- Bool - Boolean values
- Func - Functions as values
- Int - Integer values
- Ls - List values
- Meta - Types as values
- Pair - Pair values
- Prim - Primitives as values
- Reg - Registers as values

### debugging
`dump` may be used to dump any value to `stdout`.

```
  (dump [42 T Int])

[42 T Int]
[]
```

### performance
To get an idea, we will compare the opening fibonacci example with Python3.

```
$ cd bench
$ python3 fibrec.py
235
```

Around twice as slow at the moment.

```
  (func fibrec [n:Int] [Int]
    (if n.(< 2) n n.(- 1).(fibrec).(+ n.(- 2).(fibrec))))
  (bench 100 (fibrec:d 20))

[475]
```

Dropping the binding and dealing directly with the stack is slightly faster.

```
  (func fibrecs [Int] [Int]
    (if (dup).(< 2) _ (do _.(- 1) (dup).(fibrecs).(+ (swap).(- 1).(fibrecs)))))
  (bench 100 (fibrecs:d 20))

[420]
```

Let's switch to a tail recursive implementation and increase the number of repetitions to get more data.

``` 
$ cd bench
$ python3 fibtail.py
104
```

`:t` may be used to trigger TCO and avoid blowing the call stack, the resulting code runs 3 times as slow as Python3.

```
  (func fibtail [n:Int a:Int b:Int] [Int]
    (if n.(= 0) a (if n.(= 1) b (fibtail:t n.(- 1) b a.(+ b)))))
  (bench 10000 (fibtail:d 70 0 1))

[280]
```


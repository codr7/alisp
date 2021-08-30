## aLisp

### intro
aLisp aims to become a hackable, embeddable, reasonably fast interpreted custom Lisp implemented in portable C. The current version weighs in at 3 kloc and supports all features described in this document.

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
Welcome to aLisp v4

Return on empty line evaluates,
(reset) clears the stack and Ctrl+D exits.

  (func fibrec [n:Int] [Int]
    (if n.(< 2) n n.(- 1).(fibrec).(+ n.(- 2).(fibrec))))
  10.(fibrec)
[55]
```

### the stack
aLisp exposes the stack to a greater extent than what in common among Lisps.

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
  
[42]
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

Specifying a name binds the value.

```
  (func foo [n:Int] [Int] n.(+ 7))
  (foo 35)

[42]
```

### types
The following types are provided out of the box, adding more is trivial.

- Bool - Boolean values
- Func - Functions as values
- Int - Integer values
- Ls - List values
- Meta - Types as values
- Prim - Primitives as values
- Reg - Registers as values

### debugging
`dump` may be used to dump any value to `stdout`.

```
  (dump [1 T Int])

[1 T Int]
[]
```

### performance

To get an idea, we will compare the opening example with Python3.

```
  (bench 100 (fibrec:d 10))

[50]
```

And it looks like the core vm is currently around twice as slow as Python, but we're just getting started.

```
$ cd alisp
$ cd bench
$ python3 fibrec.py
22
```
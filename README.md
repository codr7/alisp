## aLisp

### intro
aLisp aims to become a hackable, embeddable, reasonably fast interpreted custom Lisp implemented in portable C. The current version weighs in at 3 kloc and supports all features described in this document.

### setup
aLisp requires CMake and a C compiler to build.

```
$ cd alisp
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./alisp
Welcome to aLisp v2

Return evaluates completed forms,
(reset) clears the stack and Ctrl+D exits.

  (func fibrec [n:Int] [Int]
    (if (< n 2) n (+ (fibrec (- n 1)) (fibrec (- n 2)))))
[]

  (fibrec 10)
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
```

`$0-9` may be used to copy the value at specified offset from the top.

```
  1 2 3 $1
[1 2 3 2]
```

`d` may be used to drop values.

```
  1 2 3 (d)
[1 2]

  (d 2)
[]
```

The stack may be thought of as the default list, separate lists may be created by surrounding code with brackets.

```
  [1 2 3]
[[1 2 3]]
```

### bindings
Values may be bound to identifiers using `let`, literals are automagically bound at compile time.

```
  (let [x 35 y 7]
    (+ x y))
[42]
```

Bindings may be aliased at compile time using `alias`.

```
  Int
[Int]

  (alias Int foo)
[Int]

  foo
[Int Int]
```

### equality
`is` returns `T` if both arguments are the same value.

```
  (is [] [])
[F]
```

`=` returns `T` if both arguments are equal.

```
  (is [] [])
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

### types
The following types are provided out of the box, adding more is trivial.

- Bool - Boolean values
- Func - Functions as values
- Int - Integer values
- Ls - List values
- Meta - Types as values
- Prim - Primitives as values
- Reg - Registers as values
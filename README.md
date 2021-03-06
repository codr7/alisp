## aLisp

[![Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/andreas7/donate)

### intro
aLisp aims to become a hackable, embeddable, reasonably fast interpreted custom Lisp implemented in portable C. The current version weighs in at 6 kloc and supports all features described in this document.

### quirks
- Parens are used for calls, brackets for lists.
- Infix calls are supported with `.` between first argument and call.

### setup
aLisp requires CMake and a C compiler to build, rlwrap is highly recommended for running the REPL.

```
$ cd alisp
$ mkdir build
$ cd build
$ cmake ..
$ make
$ rlwrap ./alisp
Welcome to aLisp v15

Return on empty line evaluates,
(reset) clears the stack and Ctrl+D exits.

  (func fibrec [n:Int] [Int]
    (if n.(< 2) n n.(- 1).(fibrec).(+ n.(- 2).(fibrec))))
  10.(fibrec)
[55]
```
### binaries
Prebuilt binaries for macOS may be found [here](https://github.com/codr7/alisp/blob/main/bin).

### the stack
aLisp exposes the stack to a greater extent than usual among Lisps.

Values are pushed on the stack.

```
  1 2 3
  
[1 2 3]
```

`_` may be specified where values are expected to pop the stack.

```
  42 (if _ T F)
  
[T]
```

`d` may be used to drop values.

```
1 2 3 4 5

[1 2 3 4 5]

  (d)

[1 2 3 4]

  (d 1)

[1 2 4]

  (d 1 2)

[4]
```

`d` may alternatively be specified as a call flag to drop returned values.

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
### scripts
`alisp` will load and evaluate the contents of its first argument if any.

./test.alisp
```
42.(dump)
```
```
$ ./alisp test.alisp
42
```

`include` may be used to inline the contents of additional scripts.

```
  "test.alisp".(include)

42
[]
```

### bindings
Values may be bound to identifiers at runtime using `let`.

```
  (let [x 35 y 7]
    x.(+ y))
    
[42]
```

`def` evaluates and binds the value at compile time.

```
  (def foo 35.(+ 7))
  foo

[42]
```

Which means it may be used to create type aliases and more.

```
  (def Foo Int)
  Foo
  
[Int]
```

Bindings may be removed using `unbind`.

```
  +

[Func(+)]

  +.(unbind) +

Unknown id: +
```

### equality
`is` returns `T` if both arguments are the same value.

```
  [].is([])
  
[F]
```

`=` is more relaxed and returns `T` if its arguments are deep equal.

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

### fixpoints
Fixpoint literals are scaled to the specified number of decimals.

```
  1.5

[1.5]
```

Addition and subtraction keep the maximum operand scale.

```
  1.5.(+ 0.75)

[2.25]
```

`fix` may be used to convert integers to fixpoints.

```
  (fix 42 2)

[42.00]
```

Fixpoints may be converted to integers using `int`.

```
  (fix 42 2).(int)

[42]
```

`scale-of` may be used to get the scale of a fixpoint.

```
  2.25.(scale-of)

[2]
```

`trunc` and `frac` may be used to get integral and fractional parts.

```
  2.25.(trunc)

[2.00]
```
```
  2.25.(frac)

[0.25]
```

### floats
`float` may be used to convert integers and fixpoints to floats.

```
  42.(float)

[42.000000]
```

Floats may be converted to integers using `int`.

```
  42.(float).(int)

[42]
```

Dividing integers results in a float.

```
  1.(/ 2)
[0.500000]
```

### strings
Strings are immutable and unique, two strings with the same contents will always be the same value.

```
  "foo".(is "foo")

[T]
```

### symbols
Quoted identifiers are represented as symbols.

```
  + '+
[Multi(+) '+]
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
  []

[NIL]

  [1]

[NIL 1:NIL]

  [1 2]

[NIL 1:NIL 1:2]

  [1 2 3]

[NIL 1:NIL 1:2 1:2:3]
```

The canonical tail recursive transformation goes something like this:

```
  (func my-map [in:List tr:Target] [List]
    (func helper [in:Any out:Any] [List]
      (if in.(nil?)
        out.(reverse)
        (helper:t in.(tail) tr.(in.(head)):out)))
    (helper:t in NIL))

  [1 2 3].(my-map (\ [Int] [Int] _.(+ 1)))

[2:3:4]
```
### iterators
`for` may be used to iterate any sequence.

```
  (for 3)

[0 1 2]
```

When a variable is specified, it is automatically bound for each iteration.

```
  (for i:[1 3 5] i)

[1 3 5]
```

`break` may be used to end the loop prematurely, any arguments are simply pushed on the stack.

```
  (for 3 (if (dup).(is 1) (break)))

[0 1]
```

`map` may be used to apply specified function to specified sequence, it returns a lazy iterator.

```
  (map (\ [Int] [Int] _.(+ 7)) [1 2 3])

[Iter(0x7fd2925e6020)]

  (for _)

[8 9 10]
```

### functions
New functions may be defined using `func`.

```
  (func foo [] [Int] 42)
  (foo)
  
[42]
```

Functions are lexically scoped.

```
  (func foo [] []
    (func bar [] [Int] 42)
    (bar))

[]

  (foo)

[42]

  (bar)

Unknown call target: bar
```

Functions capture their environment.

```
  (let [bar 42]
    (func foo [] [Int] bar)
    foo)
  
[42]
```

Anonymous arguments are left on the stack.

```
  (func foo [Int] [Int] _.(+ 7))
  35.(foo)

[42]
```

Named arguments are bound before evaluating the body.

```
  (func foo [n:Int] [Int] n.(+ 7))
  35.(foo)

[42]
```

### multiple dispatch
When multiple function definitions share the same name, the most specific one is called.

```
  (func foo [x:Int] [Meta] Int)
  (func foo [x:Any] [Meta] Any)

  42.(foo)
  T.(foo)

[Int Any]
```

### anonymous functions
Anonymous functions may be created by simply replacing `func` and the name with `\`.

```
  (\ [] [Int] 42)

[Func(0x7fcecbc094f0)]

  _.()

[42]
```

### threads
Threads run in complete isolation as separate VM instances.

```
  (thread [Int]
    (sleep 1000)
    42)

[Thread(0x7fc6b9000020)]

  _.(join)
  
[42]
```

Each thread comes with a built-in queue.

```
  (thread [] inbox.(dump)).(join)

Queue(0x7fe86902d810)
[]
```

`send` may be used to push items to a thread's inbox.

```
  (let [t (thread [Int] inbox.(pop))]
    t.(send 42)
    t.(join))

[42]
```

Queues are iterable, which makes it convient to implement worker threads as for loops.

```
  (let [t (thread [Int] (for m:inbox (if m.(is 'stop) (break) m.(dump))))]
    t.(send 'foo)
    t.(send 'bar)
    t.(send 'stop)
    t.(join))

'foo
'bar

[]
```

### types
The following types are provided out of the box, adding more is trivial.

- Any - Any value
- Bool: Any - Boolean values
- Fix: Num - Fix point values
- Float: Num - Floating point values
- Func: Target - Functions as values
- Int: Num Seq - Integer values
- Iter: Seq - Iterator values
- List: Seq - List values
- Meta: Any - Types as values
- Multi: Target - Dispatchers as values
- Nil: List - Missing values
- Num: Any - Numeric values
- Pair: List - Pair values
- Prim: Any - Primitives as values
- Queue: Seq - Queue values
- Reg: Any - Registers as values
- Seq: Any - Sequence values
- String: Seq - String values
- Sym: Any - Quoted identifiers
- Target: Any - Callable values
- Thread: Any - Threads as values

### debugging
`dump` may be used to dump any value to `stdout`.

```
  (dump [42 T Int])

[42 T Int]
[]
```

### testing

`test` runs the body and checks the stack against the specified suffix.

```
  (test "insanity" [4] 1.(+ 2))

Testing insanity...failure!
Expected: [4]
Actual:   [3]
[]
```

aLisp includes a modest but growing [test suite](https://github.com/codr7/alisp/blob/main/test/all.alisp).

```
$ cd alisp/build
$ ./alisp ../test/all.alisp
Testing int add...success!
Testing int sub...success!
Testing fix...success!
Testing fix add...success!
Testing fix sub...success!
Testing string is...success!
Testing int iter...success!
Testing list iter...success!
Testing func...success!
Testing func args...success!
Testing closure...success!
Testing anon func...success!
Testing tco...success!
Testing multiple dispatch...success!
Testing thread join...success!
0
```

### compile time evaluation
`ceval` may be used to evaluate forms at compile time and emit code to push their results.

```
  (func foo _ [Int] (ceval (dump "here!") 35.(+ 7)))

"here!"
[]

  (foo)
  
[42]
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

[436]
```

Dropping the binding and dealing directly with the stack is slightly faster.

```
  (func fibrec-s [Int] [Int]
    (if (dup).(< 2) _ (do _.(- 1) (dup).(fibrec-s).(+ (swap).(- 1).(fibrec-s)))))
  (bench 100 (fibrec-s:d 20))

[349]
```

Memoization is a nice solution for these kinds of problems, `:m` may be used to memoize calls.
Note that the number of repetitions increased by four orders of magnitude to get a similar measurable time.

```
  (func fibrec-m [n:Int] [Int]
    (if n.(< 2) n n.(- 1).(fibrec-m:m).(+ n.(- 2).(fibrec-m:m))))
  (bench 1000000 (fibrec-m:d 20))

[507]
```

Let's switch to a tail recursive implementation to get more data.

``` 
$ cd bench
$ python3 fibtail.py
104
```

`:t` may be used to trigger TCO and avoid blowing the call stack, the resulting code again runs roughly twice as slow as Python3.

```
  (func fibtail [n:Int a:Int b:Int] [Int]
    (if n.(is 0) a (if n.(is 1) b (fibtail:t n.(- 1) b a.(+ b)))))
  (bench 10000 (fibtail:d 70 0 1))

[213]
```

When combining TCO with memoization, only memoized frames are skipped.

```
  (func fibtail-m [n:Int a:Int b:Int] [Int]
    (if n.(is 0) a (if n.(is 1) b (fibtail-m:m:t n.(- 1) b a.(+ b)))))
  (bench 10000 (fibtail-m:d 70 0 1))

[10]
```
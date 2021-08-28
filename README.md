## aLisp

### intro
aLisp aims to become a hackable, embeddable, reasonably fast interpreted custom Lisp implemented in portable C. The current version weighs in at 3 kloc and supports all features described in this document.

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

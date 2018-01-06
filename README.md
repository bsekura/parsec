# Haskell like parser combinators in C++

This is an example of using Haskell like parser combinators in C++.
As a test, a simple json parser is implemented. Json parser is not
fully compliant - it's a simple test to demonstrate how easy it is to build
more complex parsers using parser combinators.

## build

```shell
$ mkdir build
$ cd build
$ cmake ..
```

## run

```shell
$ ../bin/parsec ../data/p.json
```

On success, it will pretty print parsed json to stdout.
It also accepts data on stdin.

## NOTES

Performance suffers due to abuse of std::function, which can be somewhat improved by e.g. using
[folly function](https://github.com/facebook/folly). Parser combinators are basically
chains of function invocations on parse input. We eliminate most of data copies by
using move semantics. Real advantange is the expressiveness, composability and readability of parsing code.
Json data object is an algebraic data type, implemented in C++ as a tagged union.

## references

[Haskell parsers tutorial](http://dev.stephendiehl.com/fun/002_parsers.html)

[Haskell parsec package](https://wiki.haskell.org/Parsec)


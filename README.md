# apollo3 Chess Engine

apollo3 is a chess engine written in C++. As the name might suggest, it is the
third generation (or attempt) of a chess engine that I have been working on
since around 2017. Apollo 1 was written in Rust and Apollo 2 was written in
Go.

Other than be a capable chess engine, a primary goal of apollo3 is to make use
of modern C++. Because of this, it requires that you have a C++ compiler capable
of handling C++17 features. apollo3 tends to be light on template metaprogramming
but does lean heavily on `constexpr` to generate move tables at compile-time.

## Building

apollo3 builds with CMake. Like many CMake projects, you can build with:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

To do a release build:

```
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

The CMake file also has defines for building with some sanitizers, if your compiler
supports them:

```
$ cmake .. -DAPOLLO_CLANG_ASAN=1 (or -DAPOLLO_CLANG_UBSAN=1)
$ make
```

If you have Doxygen installed on your system, you can build documentation using
the `doc` target:

```
$ cmake ..
$ make doc
```

## Tests

The core of Apollo (encoding the logic of Chess and rules of the game) must be as close
to bug-free as possible if we want apollo3 to play decent chess. apollo3 uses gtest to
aggressively test the core logic of the engine.

The most effective suite of tests that apollo3 is in "perft_test.cc", which runs the engine
through [PERFT](https://www.chessprogramming.org/Perft) tests of known board positions to
verify that apollo3 generates the right number of moves. This is an extremely effective
way to shake out bugs in the move generator.

The apollo3 CLI also has a PERFT subcommand for calculating PERFT numbers of a particular board
position. The `--save-intermediates` flag dumps a JSON database containing all moves for 
intermediate board positions seen while performing the PERFT search. Combined with the
`movegen_diff.py` Python script, this is a very effective way to debug move generation bugs.

## Goal

It is the goal of apollo3 to be a decent chess engine. My personal goal is that I want apollo3
to beat me in a game of chess. I am not particularly good at chess, so I believe that this goal
is not too far off!
# CROWN (Concolic testing for Real-wOrld softWare aNalysis)

## Overview of CROWN

CROWN is a lightweight easy-to-customize concolic testing tool for real-world C
programs, which is extended from CREST(https://github.com/jburnim/crest). CROWN
supports complex C features such as bitwise operators, floating point
arithmetic, and so on.

Currently, We have tested CROWN on Ubuntu 14.04 and 16.04 on x86-64. 

## How to setup CROWN

### Install Dependencies

CROWN requires a Ocaml compiler to build its instrumentation engine CIL and a 
C++ compiler to build itself. You can install the dependencies as follows:

```
$ sudo apt-get install ocaml ocaml-findlib
$ sudo apt-get install g++ build-essential gcc-multilib g++-multilib
```

### Build CIL and CROWN

```
$ cd cil/
$ ./configure && make
$ cd ../src
$ make
$ cd ..
```

## How to use CROWN

### Prepare a program for CROWN

To use CROWN on a C program, use functions SYM_int, SYM_char,
etc., declared in "crown.h", to generate symbolic inputs for your
program. For example, see the example program example/example.c

For simple, single-file programs, you can use the build script
"bin/crownc" to instrument and compile your test program.

### Run CROWN 

CROWN is run on an instrumented program as:
```
$ bin/run_crown PROGRAM NUM_ITERATIONS -STRATEGY
```
Possibly strategies include: dfs, cfg, random, uniform_random, random_input.

Example commands to test the "test/uniform_test.c" program:
```
$ cd example
$ ../bin/crownc example.c
$ ../bin/run_crown ./example 10 -dfs
```
This should produce output roughly like:

```
... [GARBAGE] ...
Read 2 branches.
Read 7 nodes.
Wrote 0 branch edges.

-------------------------
input = 0
input <= 0
Iteration 1 (0s, 0.0s): covered 1 branches [1 reach funs, 2 reach branches].(1, 0)
-------------------------
input = 1
input > 0
Iteration 2 (0s, 0.15s): covered 2 branches [1 reach funs, 2 reach branches].(2, 1)
```

License
=====

CROWN is distributed under the revised BSD license. See LICENSE for
details.

CROWN extends CREST concolic testing tool. CREST is distributed under the
revised BSD license, too. See LICENSE.CREST for details.

This distribution includes a modified version of CIL, a static library
of Z3 SMT solver.  CIL is also distributed under the revised BSD
license.  See cil/LICENSE for details. Z3 is distributed under the MIT license.
See LICENSE.Z3 for details.

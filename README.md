# F2L_Recovery

F2L_Recovery is a C++20 Rubik's Cube state engine, F2L search library, and SFML visualiser.

## Build

### Linux headless core

```bash
cmake -S . -B build -DF2L_BUILD_APP=OFF -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -D_GLIBCXX_ASSERTIONS"
cmake --build build --parallel
ctest --test-dir build --output-on-failure --no-tests=error
```

Release:

```bash
cmake -S . -B build-rel -DF2L_BUILD_APP=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel --parallel
ctest --test-dir build-rel --output-on-failure --no-tests=error
```

### Linux SFML application

Install the system libraries in `build.sh`, then run:

```bash
./build.sh
```

Or configure directly:

```bash
cmake -S . -B build-app -DF2L_BUILD_APP=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-app --parallel
./build-app/F2L_Recovery
```

### Windows

```bat
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Application commands

- `r` resets the cube.
- `scramble [n]` applies a random scramble; the default is 20 moves.
- `solve` runs F2L search in a background thread.
- `quit` closes the application.
- Any other input is parsed as cube moves.
- Input is split by whitespace. Standard moves include `U R F D L B`,
  slice moves `M E S`, rotations `x y z`, wide moves `u r f d l b`,
  primes (`R'`), doubles (`R2`), and combined double-prime notation.

## Architecture

- `src/move.hpp`: cube state representation and composition.
- `src/rubik.hpp`: base moves and strict move-string parser.
- `src/solver.hpp`: F2L goals, normalisation, macro search, verification,
  and the legacy `F2LPartialSolver` compatibility wrapper.
- `src/main.cpp`: SFML and interactive console application.
- `src/draw.cpp`: cube facelet conversion and rendering.
- `tests/test_core.cpp`: cube invariants and parser regression tests.
- `tests/test_solver.cpp`: goals, normalisation, search, and solver output tests.
- `tests/test_real_solves.cpp`: pinned reco.nz 3x3 reconstruction fixtures.
- `tests/bench_solver.cpp`: solver benchmark (not registered with CTest).

## Solver behavior

The solver uses iterative deepening over macro-moves, a move-state
transposition table, timeout/cancellation checks, and exact or F2L goal
predicates. Whole-cube rotations are normalised before F2L piece checks.

Search depth and timeout are bounded. A timeout or depth limit is a normal
negative result, not a crash or assertion failure. Returned strings are
parsed and verified before use.

## Tests

The core test suite is dependency-free and runs through CTest:

```bash
ctest --test-dir build-rel --output-on-failure --no-tests=error
```

The benchmark executable can be run directly:

```bash
./build-rel/tests/f2l_solver_bench
```

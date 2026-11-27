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

### Browser search tool (WebAssembly)

On Ubuntu, install the build and smoke-test prerequisites:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake emscripten nodejs python3
```

Then configure and build the optional browser engine:

```bash
emcmake cmake -S . -B build-wasm \
  -DF2L_BUILD_APP=OFF -DF2L_BUILD_TESTS=OFF -DF2L_BUILD_WEB=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-wasm --parallel
node tests/wasm_smoke.mjs
python3 -m http.server 8000 --directory build-wasm/web
```

Open `http://localhost:8000`. Searches run in a Web Worker and use the C++
engine compiled to WebAssembly. The page supports exact-state, F2L, per-slot,
and F2L-target searches, plus missing-F2L reconstruction from a scramble,
known cross solution, and known last-layer algorithm. Reconstruction tries
all 24 cube frames, reports the detected cross orientation, and verifies the
recovered sequence against the last layer.

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
- `unknown <seconds>` searches for the missing F2L in the pinned csTimer solve
  for the full requested positive integer duration, using its known x2-frame
  cross and last-layer algorithm. It prints each new solution that ties or
  improves the shortest macro count found so far.
- `quit` closes the application.
- Any other input is parsed as cube moves.
- Input is split by whitespace. Standard moves include `U R F D L B`,
  slice moves `M E S`, rotations `x y z`, wide moves `u r f d l b`,
  primes (`R'`), doubles (`R2`), and combined double-prime notation.

## Architecture

- `src/move.hpp`: cube state representation and composition.
- `src/rubik.hpp`: base moves and strict move-string parser.
- `src/solver.hpp`: F2L goals, normalisation, macro search, verification,
  generalized missing-F2L reconstruction, and the legacy `F2LPartialSolver`
  compatibility wrapper.
- `src/main.cpp`: SFML and interactive console application.
- `src/cube_view.hpp`: shared cube-to-facelet conversion.
- `src/draw.cpp`: SFML rendering.
- `src/wasm_bindings.cpp` and `web/`: WebAssembly bindings and browser tool.
- `tests/test_core.cpp`: cube invariants and parser regression tests.
- `tests/test_solver.cpp`: goals, normalisation, search, and solver output tests.
- `tests/test_real_solves.cpp`: pinned reco.nz and CubeDB 3x3 reconstructions,
  plus a csTimer case that searches for the missing F2L from its known cross
  and last-layer algorithm.
- `tests/test_canonical_f2l.cpp`: 41 representative reversible F2L macro
  cases pinned from the public [CFOP F2L dataset](https://github.com/andyjudson/cfop/blob/main/cfop-app/public/data/cfop-f2l.json).
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

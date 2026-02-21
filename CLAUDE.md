# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Include-What-You-Use (IWYU) is a Clang-based tool that analyzes C/C++ `#include` statements. It ensures every file includes headers for all symbols it uses (and nothing more), and prefers forward declarations over full definitions where possible. It maps private headers (e.g. `bits/stl_vector.h`) to their public equivalents (`<vector>`).

The master branch tracks Clang's main branch. Version-specific branches (e.g. `clang_20`) exist for released Clang versions.

## Build Commands

```bash
# Standalone build against pre-built LLVM/Clang
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/usr/lib/llvm-XX ..
cmake --build .

# Run unit tests
cmake --build build --target iwyu-run-unittests
# or: ctest -j 6 --test-dir build

# Run regression tests (all)
python3 run_iwyu_tests.py -- ./build/bin/include-what-you-use

# Run a single regression test
python3 run_iwyu_tests.py cxx.test_array -- ./build/bin/include-what-you-use

# List available regression tests
python3 run_iwyu_tests.py --list

# Run fix_includes.py tests
python3 fix_includes_test.py

# Run iwyu_tool.py tests
python3 iwyu_tool_test.py
```

## Architecture

### Core Pipeline

1. **`iwyu_driver.cc`** — Creates and configures a Clang compiler instance from command-line arguments.
2. **`iwyu.cc`** — Main analysis: traverses the Clang AST to determine symbol uses (full use vs. forward-declare use) and follows template instantiations.
3. **`iwyu_preprocessor.cc`** — Handles `#include`/`#ifdef` directives to build the include tree; processes IWYU pragma comments.
4. **`iwyu_include_picker.cc`** — Maps private→public headers and resolves symbols with multiple possible includes. Uses `.imp` mapping files and built-in `std_symbol_map.inc`.
5. **`iwyu_output.cc`** — Translates uses into violations, checks whether existing `#include`s cover uses, and emits diagnostics and the final summary.
6. **`iwyu_cache.cc`** — Caches expensive computations (e.g. instantiated template analysis).

### Supporting Modules

- **`iwyu_ast_util.h`** — Clang AST navigation helpers.
- **`iwyu_location_util.h`** — Source location utilities.
- **`iwyu_path_util.h`** — File path normalization.
- **`iwyu_lexer_utils.h`** — Token analysis helpers.
- **`iwyu_globals.cc`** — Global state and configuration.
- **`iwyu_verrs.cc`** — Verbose/debug logging (levels described in `iwyu_output.h`; level 7 dumps AST traversal).
- **`iwyu_regex.cc`** — Regex support (LLVM basic and ECMAScript dialects).
- **`iwyu_getopt.cc`** — Portability shim for GNU `getopt` on Windows.

### Python Tools

- **`fix_includes.py`** — Applies IWYU recommendations to source files automatically.
- **`iwyu_tool.py`** — Runs IWYU with a compilation database.

### Mapping Files (`.imp`)

YAML-like files mapping private→public headers and symbols→headers. Included for Boost, Qt, Python, and compiler intrinsics. Symbol mappings for the standard library are in `std_symbol_map.inc` (compiled into the binary).

## Test Structure

Tests live in `tests/` organized by category:
- **`tests/cxx/`** — C++ language constructs
- **`tests/c/`** — C features
- **`tests/driver/`** — Driver/compilation behavior
- **`tests/stdin/`** — STDIN input handling
- **`tests/bugs/`** — Known bugs (expected failures)

Each test is a `.cc` file with optional supporting headers. Test names derive from path: `tests/cxx/array.cc` → `cxx.test_array`.

**Reusable test headers:** `direct.h`/`indirect.h` provide `IndirectClass`; include `direct.h` to force IWYU diagnostics. Subdirectory variants in `subdir/`.

**Per-test headers:** For `x.cc`, use `x-direct.h` (includes `x-indirect.h`) and `x-indirect.h` (declares symbols). Multiple: `x-d1.h`/`x-i1.h`, `x-d2.h`/`x-i2.h`.

**Test directives** (in `.cc` files):
```cpp
// IWYU_ARGS: -I . -std=c++20 -Xiwyu --no_internal_mappings
// IWYU_REQUIRES: feature(value)
// IWYU_UNSUPPORTED: feature(value)
```

Unit tests use vendored Google Test in `unittests/`.

## Coding Conventions

- Based on Google C++ Style Guide with LLVM modifications.
- 80-char line limit, 2-space indent, 4-space continuation, Unix line endings.
- PascalCase for types/functions, snake_case for variables, UPPER_CASE for macros.
- Pointer/reference binds to type: `Foo* f;`, `Bar& b;`.
- Use `auto` sparingly — only when the type is obvious on the line.
- Prefer `static` over anonymous namespaces for internal linkage.
- Format with `git clang-format --extensions=cc,h` on changed lines only — never reformat whole files.
- Every source file needs the LLVM license header (use `iwyu-check-license-header.py` to add it).
- Test files use `.cc` extension, not `.cpp`.
- Linear git history — rebase workflow, no merge commits.

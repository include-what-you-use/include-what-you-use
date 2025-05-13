# Bugs suite #

This is a special test suite where every test case maps to an open issue at
<https://github.com/include-what-you-use/include-what-you-use/issues>.

Tests in this suite are only executed if you explicitly launch
`run_iwyu_tests.py` with the `--extra-suite=bugs` argument.

Its purpose is to reproduce reported issues with an `// IWYU_XFAIL` marker, so
that if a bug is forgotten and fixed some time in the future, its associated
test case will unexpectedly succeed, and the bugs suite will fail.

Ideally, test cases in this suite are small and self-contained, but because it
can be difficult to reduce IWYU test cases, we allow use of standard library
includes (either libstdc++ or libc++), which expands the range of what we can
easily reproduce. These tests will primarily be run in our CI on a Linux system
with GCC and Clang installed. Wider portability is not a goal for these tests --
rather, it's easier to reproduce bugs if the environment is fairly constrained.

The test guidelines in [the style guide](../../docs/IWYUStyle.md) don't
necessarily apply here. We don't use reusable headers, we don't avoid system
headers and we don't force diagnostics.

The smallest testcase looks something like:

```
//===--- 1000.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include <cmath>

static void f() {
  (void)std::abs(-12);
}

/**** IWYU_SUMMARY

(tests/bugs/1000/1000.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
```

First off, the name must be the same as the issue number, so we can easily
cross-reference issues and test cases. Because most bugs require more than one
file, we create a directory for each test case, and a main test input file in
there, both with the same name -- e.g. `999/999.c`.

We allow both `*.c` and `*.cc` files as test inputs.

There are three required components:

* The license header
* The `// IWYU_XFAIL` marker, because the rest of the file represents something
  that should work, but doesn't.
* The `/**** IWYU_SUMMARY` section, otherwise there is nothing for the test
  framework to verify

You may add an `// IWYU_ARGS: ...` directive if you need to run IWYU with
arguments other than the default, and possibly `// IWYU:` diagnostic assertions
if you expect IWYU to complain about some construct.


## Working with bug repros ##

The process for creating new tests in the bugs suite is a bit more relaxed than
for the normal regression suites (which try to be fully portable, still work in
progress).

At this point we accept standard library dependencies (tests will run on a
system with both libstdc++ and libc++ available), but we can't accommodate other
library dependencies. We also don't have Windows or macOS environments at this
time.

If your test only surfaces a bug under a specific standard library
implementation, use `// IWYU_ARGS: -stdlib=libc++` or `libstdc++` to force the
test to run with said standard lib.

Once merged, a bug test will run in CI and will fail the pipeline _if the bug is
fixed_ -- this helps the IWYU project keep its bug backlog up to date.

When a bug is fixed, it's expected that a reduced test case without library
dependencies exists in one of the main suites to cover the same scenario. At
this point, the bug test case can, and should, be removed.

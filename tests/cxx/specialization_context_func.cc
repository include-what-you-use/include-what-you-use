//===--- specialization_context_func.cc - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_context_func.imp

// Tests that a specialization type reached from the body of a lambda isn't
// taken for a repeat of its traversal from the enclosing function: the
// __PRETTY_FUNCTION__ in it shows the function's enum template argument
// only for the latter. Color, W1 and W2 each map to two public headers, and
// which one Color gets depends on how often it is used: reporting Color
// once more would make "-b.h" (for Color and W1) win over "-c.h" (for W1 and
// W2).

#include "tests/cxx/specialization_context_func-direct.h"
#include "tests/cxx/specialization_context_func-defs.h"

// IWYU: Color is...*specialization_context_func-a.h
// IWYU: Color needs a declaration
void Use(S& s) { s.F<Color::kBlue>(); }
// IWYU: W1 is...*specialization_context_func-c.h
int w1 = sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1);
// IWYU: W2 is...*specialization_context_func-c.h
int w2 = sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2);

/**** IWYU_SUMMARY

tests/cxx/specialization_context_func.cc should add these lines:
#include "tests/cxx/specialization_context_func-a.h"
#include "tests/cxx/specialization_context_func-c.h"

tests/cxx/specialization_context_func.cc should remove these lines:
- #include "tests/cxx/specialization_context_func-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_context_func.cc:
#include "tests/cxx/specialization_context_func-a.h"  // for Color
#include "tests/cxx/specialization_context_func-c.h"  // for W1, W2
#include "tests/cxx/specialization_context_func-defs.h"  // for S

***** IWYU_SUMMARY */

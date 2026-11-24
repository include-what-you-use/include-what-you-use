//===--- specialization_context_call.cc - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_context_call.imp

// Tests that a specialization type reached in a call isn't taken for
// a repeat of its traversal outside of any call, or the other way around:
// a conversion inside it needs the converted-to type's full type only
// outside of calls whose callee provides it. Conv, W1 and W2 each map to two
// public headers, and which one Conv gets depends on how often it is used:
// reporting Conv more often would make "-b.h" (for Conv and W1) win over
// "-c.h" (for W1 and W2).

#include "tests/cxx/specialization_context_call-direct.h"
#include "tests/cxx/specialization_context_call-defs.h"

// IWYU: Conv needs a declaration
// IWYU: Conv is...*specialization_context_call-a.h
// IWYU: Src needs a declaration
Holder<Conv, Src> h;
// IWYU: W1 is...*specialization_context_call-c.h
int w1a = sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1);
// IWYU: W1 is...*specialization_context_call-c.h
int w1b = sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1);
// IWYU: W1 is...*specialization_context_call-c.h
int w1c = sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1);
// IWYU: W2 is...*specialization_context_call-c.h
int w2a = sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2);
// IWYU: W2 is...*specialization_context_call-c.h
int w2b = sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2);

/**** IWYU_SUMMARY

tests/cxx/specialization_context_call.cc should add these lines:
#include "tests/cxx/specialization_context_call-a.h"
#include "tests/cxx/specialization_context_call-c.h"
struct Src;

tests/cxx/specialization_context_call.cc should remove these lines:
- #include "tests/cxx/specialization_context_call-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_context_call.cc:
#include "tests/cxx/specialization_context_call-a.h"  // for Conv
#include "tests/cxx/specialization_context_call-c.h"  // for W1, W2
#include "tests/cxx/specialization_context_call-defs.h"  // for Holder
struct Src;

***** IWYU_SUMMARY */

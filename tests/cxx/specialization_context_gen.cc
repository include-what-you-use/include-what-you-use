//===--- specialization_context_gen.cc - test input -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_context_gen.imp

// Tests that a specialization type reached again after a variable template
// was traversed isn't taken for a repeat of its earlier traversal: the
// variable template's pattern makes more nodes ignored (here, the call to
// Helper). Helper, W1 and W2 each map to two public headers, and which one
// Helper gets depends on how often it is used: reporting Helper once more
// would make "-b.h" (for Helper and W1) win over "-c.h" (for W1 and W2).

#include "tests/cxx/specialization_context_gen-direct.h"
#include "tests/cxx/specialization_context_gen-defs.h"
#include "tests/cxx/specialization_context_gen-payload.h"

// IWYU: Helper() is...*specialization_context_gen-a.h
Holder<Payload> h;
// IWYU: W1 is...*specialization_context_gen-c.h
int w1 = sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1) + sizeof(W1);
// IWYU: W2 is...*specialization_context_gen-c.h
int w2 = sizeof(W2) + sizeof(W2) + sizeof(W2) + sizeof(W2);

/**** IWYU_SUMMARY

tests/cxx/specialization_context_gen.cc should add these lines:
#include "tests/cxx/specialization_context_gen-a.h"
#include "tests/cxx/specialization_context_gen-c.h"

tests/cxx/specialization_context_gen.cc should remove these lines:
- #include "tests/cxx/specialization_context_gen-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_context_gen.cc:
#include "tests/cxx/specialization_context_gen-a.h"  // for Helper
#include "tests/cxx/specialization_context_gen-c.h"  // for W1, W2
#include "tests/cxx/specialization_context_gen-defs.h"  // for Holder
#include "tests/cxx/specialization_context_gen-payload.h"  // for Payload

***** IWYU_SUMMARY */

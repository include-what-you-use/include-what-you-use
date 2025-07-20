//===--- namespace_use.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that using-directives i.e. using namespace, get appropriate include
// suggestions.

#include "tests/cxx/namespace_use-d1.h"

// Purely using the namespace, not any symbols.
// This could be unused, or being used transitively in namespace lookup.
// The include suggestion will be the first header that declares the namespace.
// IWYU: i1_ns is...*namespace_use-i1.h
using namespace i1_ns;

// This namespace gets used for symbols within it.
// Suggestions for includes will come via the symbol usage rather than here.
using namespace i2_ns;

int main(int, const char**) {
  // IWYU: i2_ns::GetValue is...*namespace_use-i2.h
  return GetValue();
}

/**** IWYU_SUMMARY

tests/cxx/namespace_use.cc should add these lines:
#include "tests/cxx/namespace_use-i1.h"
#include "tests/cxx/namespace_use-i2.h"

tests/cxx/namespace_use.cc should remove these lines:
- #include "tests/cxx/namespace_use-d1.h"  // lines XX-XX

The full include-list for tests/cxx/namespace_use.cc:
#include "tests/cxx/namespace_use-i1.h"  // for i1_ns
#include "tests/cxx/namespace_use-i2.h"  // for GetValue

***** IWYU_SUMMARY */

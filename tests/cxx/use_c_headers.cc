//===--- use_c_headers.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-user-defined-literals -Xiwyu --use_c_headers

// Tests --use_c_headers option which forces suggestions of C standard library
// headers (<name.h>) instead of their C++ counterparts (<cname>).

#include "tests/cxx/std_symbol_mapping-direct.h"
#include <cassert>

// IWYU: int8_t is...*<stdint.h>
int8_t int8_t_var;
// If std:: qualification is written explicitly, <cname> header should
// nevertheless be suggested.
// IWYU: std::size_t is...*<cstddef>
std::size_t s;

void Fn() {
  // IWYU: assert is...*<assert.h>
  assert(true);
}

/**** IWYU_SUMMARY

tests/cxx/use_c_headers.cc should add these lines:
#include <assert.h>
#include <stdint.h>
#include <cstddef>

tests/cxx/use_c_headers.cc should remove these lines:
- #include <cassert>  // lines XX-XX
- #include "tests/cxx/std_symbol_mapping-direct.h"  // lines XX-XX

The full include-list for tests/cxx/use_c_headers.cc:
#include <assert.h>  // for assert
#include <stdint.h>  // for int8_t
#include <cstddef>  // for size_t

***** IWYU_SUMMARY */

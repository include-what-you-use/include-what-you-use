//===--- pch_inc_code.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test IWYU's handling of precompiled headers explicitly included in code.
// This is a pattern popularized by MSVC and allowed by GCC, where an include
// directive is used as a marker to signal that a precompiled header should be
// pulled into the translation unit.
//
// Under these compilers' rules, the precompiled header must be included first.
//
// A PCH lives under special rules:
// - should never be removed
// - all headers included in it are considered prefix headers
// - always stays the first include, no matter what changes IWYU makes
//
// This test assumes being run with --prefix_header_includes=remove and
// --pch_in_code.

#include "tests/cxx/pch.h"  // this is the precompiled header
#include <stdlib.h>  // unused
#include <stdint.h>  // for int8_t
#include "tests/cxx/indirect.h"

IndirectClass ic;
int8_t global_byte;

/**** IWYU_SUMMARY

tests/cxx/pch_in_code.cc should add these lines:

tests/cxx/pch_in_code.cc should remove these lines:
- #include <stdlib.h>  // lines XX-XX
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/pch_in_code.cc:
#include "tests/cxx/pch.h"
#include <stdint.h>  // for int8_t

***** IWYU_SUMMARY */

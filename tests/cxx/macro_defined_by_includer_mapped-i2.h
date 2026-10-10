//===--- macro_defined_by_includer_mapped-i2.h - test input file for iwyu -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU pragma: private, include "tests/cxx/macro_defined_by_includer_mapped-d2.h"

// Private to -d2.h by pragma.  Like glibc <bits/*.h>, it refuses to be
// included other than through its public header.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_MAPPED_D2_H_
#error "Include macro_defined_by_includer_mapped-d2.h instead."
#endif

struct PragmaPrivateBase {};

#ifdef PRAGMA_PRIVATE_FEATURE
struct PragmaPrivateFeature {};
#endif

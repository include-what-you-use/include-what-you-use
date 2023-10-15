//===--- ptr_ref_aliases-d2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/ptr_ref_aliases-i1.h"

// IWYU: Indirect is...*ptr_ref_aliases-i2.h
using ProvidingPtrAlias = Indirect*;
// IWYU: Indirect is...*ptr_ref_aliases-i2.h
using ProvidingRefAlias = Indirect&;

/**** IWYU_SUMMARY

tests/cxx/ptr_ref_aliases-d2.h should add these lines:
#include "tests/cxx/ptr_ref_aliases-i2.h"

tests/cxx/ptr_ref_aliases-d2.h should remove these lines:
- #include "tests/cxx/ptr_ref_aliases-i1.h"  // lines XX-XX

The full include-list for tests/cxx/ptr_ref_aliases-d2.h:
#include "tests/cxx/ptr_ref_aliases-i2.h"  // for Indirect

***** IWYU_SUMMARY */

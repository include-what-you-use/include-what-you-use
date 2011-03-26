//===--- precomputed_tpl_args.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the precomputed template-arg-use list in iwyu_cache.cc.

#include <vector>
#include <set>
#include "tests/precomputed_tpl_args-d1.h"

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
std::vector<IndirectClass> ic_vec;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*precomputed_tpl_args-i1.h
std::set<IndirectClass> ic_set;

// This class provides a specialization of less that we should see.
// IWYU: SpecializationClass needs a declaration
// IWYU: SpecializationClass is...*precomputed_tpl_args-i1.h
// IWYU: std::less is...*precomputed_tpl_args-i1.h
std::set<SpecializationClass> sc_set;


/**** IWYU_SUMMARY

tests/precomputed_tpl_args.cc should add these lines:
#include "tests/precomputed_tpl_args-i1.h"

tests/precomputed_tpl_args.cc should remove these lines:
- #include "tests/precomputed_tpl_args-d1.h"  // lines XX-XX

The full include-list for tests/precomputed_tpl_args.cc:
#include <set>  // for set
#include <vector>  // for vector
#include "tests/precomputed_tpl_args-i1.h"  // for IndirectClass, SpecializationClass, less

***** IWYU_SUMMARY */

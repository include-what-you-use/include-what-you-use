//===--- fwd_decl_mapping_path_local.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/fwd_decl_mapping_path_local.imp

// Tests forward-declaration symbol mapping in the case of relative quoted
// includes. '-I .' command line argument is left out intentionally.

#include "fwd_decl_mapping_path_local-d1.h"

// A forward-declaration of Class is mapped to '*-i2.h', which in turn is
// exported by '*-i1.h', so including '*-i1.h' should be enough.
Class* pClass;

// Make sure '*-i1.h' is to be reported.
// IWYU: ClassFromI1 is...*-i1.h
ClassFromI1 cfi1;

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_mapping_path_local.cc should add these lines:
#include "fwd_decl_mapping_path_local-i1.h"

tests/cxx/fwd_decl_mapping_path_local.cc should remove these lines:
- #include "fwd_decl_mapping_path_local-d1.h"  // lines XX-XX

The full include-list for tests/cxx/fwd_decl_mapping_path_local.cc:
#include "fwd_decl_mapping_path_local-i1.h"  // for Class (ptr only), ClassFromI1

***** IWYU_SUMMARY */

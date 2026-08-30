//===--- no_fwd_decls_src_only.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --no_fwd_decls=src-only -I .

#include "tests/cxx/no_fwd_decls_src_only.h"

void FwdDeclFinal::testFinalTemplate(FinalTemplate<int>* finalTemplate) {
}

void FwdDeclFinal::testFinalClass(FinalClass* finalClass) {
}

/**** IWYU_SUMMARY

tests/cxx/no_fwd_decls_src_only.cc should add these lines:
#include "tests/cxx/no_fwd_decls_src_only-d1.h"

tests/cxx/no_fwd_decls_src_only.cc should remove these lines:

The full include-list for tests/cxx/no_fwd_decls_src_only.cc:
#include "tests/cxx/no_fwd_decls_src_only.h"
#include "tests/cxx/no_fwd_decls_src_only-d1.h"  // for FinalClass, FinalTemplate

***** IWYU_SUMMARY */

//===--- fwd_decl_final.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/fwd_decl_final.h"

void FwdDeclFinal::testFinalTemplate(FinalTemplate<int>* finalTemplate) {
}

void FwdDeclFinal::testFinalClass(FinalClass* finalClass) {
}

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_final.cc should add these lines:
class FinalClass;
template <typename T> class FinalTemplate;

tests/cxx/fwd_decl_final.cc should remove these lines:

The full include-list for tests/cxx/fwd_decl_final.cc:
#include "tests/cxx/fwd_decl_final.h"
class FinalClass;
template <typename T> class FinalTemplate;

***** IWYU_SUMMARY */

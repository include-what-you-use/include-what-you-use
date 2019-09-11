//===--- fwd_decl_final.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_H_

#include "tests/cxx/fwd_decl_final-d1.h"

class FwdDeclFinal {
public:
  void testFinalTemplate(FinalTemplate<int>* finalTemplate);
  void testFinalClass(FinalClass* finalClass);
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_FWD_DECL_FINAL_H_

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_final.h should add these lines:
class FinalClass;
template <typename T> class FinalTemplate;

tests/cxx/fwd_decl_final.h should remove these lines:
- #include "tests/cxx/fwd_decl_final-d1.h"  // lines XX-XX

The full include-list for tests/cxx/fwd_decl_final.h:
class FinalClass;
template <typename T> class FinalTemplate;

***** IWYU_SUMMARY */

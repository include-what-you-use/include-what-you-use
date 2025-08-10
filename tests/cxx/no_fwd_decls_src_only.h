//===--- no_fwd_decls_src_only.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS_SRC_ONLY_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS_SRC_ONLY_H_

#include "tests/cxx/no_fwd_decls_src_only-d1.h"

class FwdDeclFinal {
public:
  void testFinalTemplate(FinalTemplate<int>* finalTemplate);
  void testFinalClass(FinalClass* finalClass);
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_NO_FWD_DECLS_SRC_ONLY_H_

/**** IWYU_SUMMARY

tests/cxx/no_fwd_decls_src_only.h should add these lines:
class FinalClass;
template <typename T> class FinalTemplate;

tests/cxx/no_fwd_decls_src_only.h should remove these lines:
- #include "tests/cxx/no_fwd_decls_src_only-d1.h"  // lines XX-XX

The full include-list for tests/cxx/no_fwd_decls_src_only.h:
class FinalClass;
template <typename T> class FinalTemplate;

***** IWYU_SUMMARY */

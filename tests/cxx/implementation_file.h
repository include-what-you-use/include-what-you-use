//===--- implementation_file.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_H_

#include "tests/cxx/indirect.h"

class ImplFileClass {
 public:
  void InlineMethod();
  void DetailMethod();
  template <typename T>
  void TemplateMethod(T t);

 private:
  class Nested;

  IndirectClass member;
};

int ImplFileFunction(int i);

template <typename T>
T ImplFileTemplate(T t);

// Not used by this file: kept anyway, since it defines what this file
// declares.  Kept as written too, although IWYU would spell it
// "tests/cxx/implementation_file-impl.icc".
#include "implementation_file-impl.icc"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_H_

/**** IWYU_SUMMARY

(tests/cxx/implementation_file.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

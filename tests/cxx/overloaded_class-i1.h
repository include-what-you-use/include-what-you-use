//===--- overloaded_class-i1.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_OVERLOADED_CLASS_I1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_OVERLOADED_CLASS_I1_H_

#include <vector>

// This is a template function that instantiates a template type.  It
// should be responsible for the type because all possible
// instantiations come from the same file.

template<class T> void MyFunc() {
  std::vector<T> v;
  v.resize(10);
}

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_OVERLOADED_CLASS_I1_H_

/**** IWYU_SUMMARY

(tests/cxx/overloaded_class-i1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

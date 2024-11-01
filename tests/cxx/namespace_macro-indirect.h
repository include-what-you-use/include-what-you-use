//===--- namespace_macro-indirect.h - test input file for IWYU ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_MACRO_INDIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_MACRO_INDIRECT_H_

#ifndef MYNS
#define MYNS myns
#endif

namespace MYNS {

void foo() {
  // do something
}

}  // namespace MYNS

#endif

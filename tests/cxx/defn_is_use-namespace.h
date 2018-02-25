//===--- defn_is_use-namespace.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFN_IS_USE_NAMESPACE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFN_IS_USE_NAMESPACE_H_

namespace ns1 {
namespace ns2 {
  void Foo();
}
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFN_IS_USE_NAMESPACE_H_

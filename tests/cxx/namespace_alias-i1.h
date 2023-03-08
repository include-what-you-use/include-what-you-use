//===--- namespace_alias-i1.h - test input file for IWYU ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_I1_H_

// Just declare something in a namespace. Nested for good measure.
namespace ns1 {
namespace ns2 {
void Function1();
}
}  // namespace ns1

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_NAMESPACE_ALIAS_I1_H_

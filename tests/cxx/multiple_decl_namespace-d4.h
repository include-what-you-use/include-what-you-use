//===--- multiple_decl_namespace-d4.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D4_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D4_H_

// Namespace where all full declarations are spread across files. It
// contains interfaces where some types will need to be forward
// declared by users.
namespace test::multiple_header_ns {

class Class4 {
  int i1;
  int i2;
};

Class4* Function4a();
void Function4b();
}  // namespace test::multiple_header_ns

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D4_H_

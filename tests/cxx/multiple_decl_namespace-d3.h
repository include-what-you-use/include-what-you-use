//===--- multiple_decl_namespace-d3.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D3_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D3_H_

// Namespace where all full declarations are in a single file. It
// contains interfaces where some types will need to be forward
// declared by users.
//
// Split the namespace up into a few declarations for test coverage.
namespace test::single_header_ns {
class Class3 {
  int i1;
  int i2;
};
}  // namespace test::single_header_ns

namespace test::single_header_ns {
Class3* Function3a();
}  // namespace test::single_header_ns

namespace test::single_header_ns {
void Function3b();
}  // namespace test::single_header_ns

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D3_H_

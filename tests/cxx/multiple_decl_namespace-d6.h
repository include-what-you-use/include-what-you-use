//===--- multiple_decl_namespace-d6.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D6_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D6_H_

// This file contains further declarations for our multiple_header_*
// namespaces.
namespace test::multiple_header_ns {
void Function6a();
void Function6b();
}  // namespace test::multiple_header_ns

namespace test::multiple_header_with_using_ns {
void Function6c();
void Function6d();
}  // namespace test::multiple_header_with_using_ns
#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D6_H_

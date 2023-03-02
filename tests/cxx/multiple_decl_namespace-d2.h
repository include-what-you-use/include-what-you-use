//===--- multiple_decl_namespace-d2.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D2_H_

#include "tests/cxx/multiple_decl_namespace-d1.h"  // for Flags

// Simple namespace that users don't need to forward declare anything from.
// However it uses a type from another namespace...
namespace test::simple_ns {
void Function2a(test::ns1::Flags f);
void Function2b();
}  // namespace test::simple_ns

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D2_H_

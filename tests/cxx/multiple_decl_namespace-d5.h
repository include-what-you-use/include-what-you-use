//===--- multiple_decl_namespace-d5.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D5_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D5_H_

// Dummy namespace to be referenced by using directive
namespace other::consts {
typedef unsigned Consts;
const Consts C_PI = 314;
}  // namespace other::consts

// This namespace has a using-directive. This is a pretty clear sign
// that this namespace block is not a bunch of forward declarations,
// and need to be explicitly included by users.
namespace test::multiple_header_with_using_ns {
class Class5 {
  int i1;
  int i2;
};

// A using directive makes the target ns available in the nearest common parent
// ns. This makes the content of consts available in the global namespace.
using namespace other::consts;

Class5* Function5a();
void Function5b();
void Function5c(Consts c);
}  // namespace test::multiple_header_with_using_ns

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D5_H_

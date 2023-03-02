//===--- multiple_decl_namespace-d1.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D1_H_

// This header forward declares a bunch of stuff from other
// namespaces.  None of these namespace declarations should be used as
// the 'source' of any of these namespaces.  For the purposes of
// testing, this header should be included first (so these are the
// first namespace declarations for each namespace).

// Forward declarations
namespace test::single_header_ns {
class Class3;
}
namespace test::multiple_header_ns {
class Class4;
}
namespace test::multiple_header_with_using_ns {
class Class5;
}

namespace test::ns1 {

// To be representative, this header needs to contains something that
// another namespace wants to use. Declare some flags.
typedef unsigned Flags;
const Flags F_None = 0;
const Flags F_All = 1;

// This is not referenced by the test but requires the forward declarations.
void Function1(const test::single_header_ns::Class3&,
               const test::multiple_header_ns::Class4&,
               const test::multiple_header_with_using_ns::Class5&, Flags f);
}  // namespace test::ns1

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_D1_H_

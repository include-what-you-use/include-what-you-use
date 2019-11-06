//===--- inline_namespace.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_H_

#include "tests/cxx/inline_namespace-d1.h"

// A forward-declare would typically be enough here, but the presence of the
// inline namespace xyz::v1 disqualifies forward declaration, and promotes it to
// a full use.

// IWYU: xyz::Foo is...*inline_namespace-i1.h
int Function(const xyz::Foo& foo);

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_INLINE_NAMESPACE_H_

/**** IWYU_SUMMARY

tests/cxx/inline_namespace.h should add these lines:
#include "tests/cxx/inline_namespace-i1.h"

tests/cxx/inline_namespace.h should remove these lines:
- #include "tests/cxx/inline_namespace-d1.h"  // lines XX-XX

The full include-list for tests/cxx/inline_namespace.h:
#include "tests/cxx/inline_namespace-i1.h"  // for Foo

***** IWYU_SUMMARY */

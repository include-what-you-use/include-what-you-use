//===--- block_template_internals.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/block_template_internals.imp -I .

// Test that IWYU does not report template implementation details, especially
// for templates from external libraries.

#include "tests/cxx/block_template_internals-d1.h"

// IWYU should not report InternalFn here. It is assumed to be provided
// by the public header for TplFn.
int x = TplFn(1);

// No need to report the complete definition of BaseTpl here: it is provided
// by DerivedTpl, and the second template parameter is not used. For this test
// case, it is assumed that a template argument coincides with the template base
// class just accidentally. (In fact, it is introduced only to allow BaseTpl to
// be reported from InstantiatedTemplateVisitor.)
// IWYU: BaseTpl needs a declaration
auto s1 = sizeof(DerivedTpl<int, BaseTpl<int>>);
// TODO: the same logic should better be applied for e.g. constructor handling,
// like in this case:
// DerivedTpl<int, BaseTpl<int>> t;

/**** IWYU_SUMMARY

tests/cxx/block_template_internals.cc should add these lines:
template <typename> class BaseTpl;

tests/cxx/block_template_internals.cc should remove these lines:

The full include-list for tests/cxx/block_template_internals.cc:
#include "tests/cxx/block_template_internals-d1.h"  // for DerivedTpl, TplFn
template <typename> class BaseTpl;

***** IWYU_SUMMARY */

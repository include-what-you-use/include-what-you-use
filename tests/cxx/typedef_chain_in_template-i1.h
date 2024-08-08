//===--- typedef_chain_in_template-i1.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header mimics ext/alloc_traits.h in libstdc++.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_IN_TEMPLATE_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_IN_TEMPLATE_I1_H_

template<typename T>
struct TypedefWrapper {
  typedef T value_type;
  typedef value_type& reference;
};

template <typename T>
using IdentityAlias2 = T;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_IN_TEMPLATE_I1_H_

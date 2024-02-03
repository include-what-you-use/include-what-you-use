//===--- typedef_chain_in_template-d5.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/typedef_chain_in_template-i1.h"
#include "tests/cxx/typedef_chain_in_template-i2.h"

template <typename T>
struct IdentityStructComplex {
  using Type = typename TypedefWrapper<T>::value_type;
};

template <typename T>
using IdentityAlias = T;

template <typename T>
using IdentityAliasComplex = IdentityAlias<T>;

template <typename T>
struct Tpl {
  static constexpr auto s = sizeof(T);
};

using TplWithNonProvidedAliased1 = Tpl<NonProvidingAlias>;
using TplWithNonProvidedAliased2 = Tpl<NonProvidingAliasTpl<1>>;

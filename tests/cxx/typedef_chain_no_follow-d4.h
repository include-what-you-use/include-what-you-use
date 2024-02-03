//===--- typedef_chain_no_follow-d4.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/typedef_chain_class.h"

template <typename T>
using IdentityAlias = T;

using ProvidingWithAliasTpl = IdentityAlias<TypedefChainClass>;

template <typename T>
struct IdentityStruct {
  using Type = T;
};

using ProvidingWithStructTpl = IdentityStruct<TypedefChainClass>::Type;

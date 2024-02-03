//===--- typedef_chain_in_template-i2.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class TypedefChainClass;

using NonProvidingAlias = TypedefChainClass;

template <int>
using NonProvidingAliasTpl = TypedefChainClass;

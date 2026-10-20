//===--- default_tpl_arg_pack_expansion-i1.h - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_PACK_EXPANSION_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_PACK_EXPANSION_I1_H_

template <typename A, typename B>
struct Pair {};

template <typename A, typename B, typename C>
struct Triple {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_DEFAULT_TPL_ARG_PACK_EXPANSION_I1_H_

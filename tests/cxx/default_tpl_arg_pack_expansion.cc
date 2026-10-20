//===--- default_tpl_arg_pack_expansion.cc - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests which declarations IWYU reports for template specializations written
// with a pack expansion. 'Triple<Ts...>' doesn't say whether the default
// argument of 'Triple' is used until 'Ts' is known, so in a template
// definition IWYU reports a use of any declaration of 'Triple', not of the
// redeclaration in -i2.h that gives the default.

#include "tests/cxx/default_tpl_arg_pack_expansion-d1.h"

template <typename T>
struct Trait;

template <typename... Ts>
// IWYU: Pair needs a declaration
struct Trait<Pair<Ts...>> {};

template <typename... Ts>
struct Holder {
  // IWYU: Triple needs a declaration
  Triple<Ts...>* p;
};

// TODO: this instantiation uses the default argument of 'Triple', so IWYU
// should require -i2.h, rather than suggest removing the only include that
// provides it.
Holder<char, long> two_args;

template <typename... Ts>
// IWYU: Triple needs a declaration
using TripleAlias = Triple<Ts...>;

// TODO: as above, this uses the default argument of 'Triple'.
TripleAlias<char, long>* alias;

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg_pack_expansion.cc should add these lines:
template <typename A, typename B, typename C> struct Triple;
template <typename A, typename B> struct Pair;

tests/cxx/default_tpl_arg_pack_expansion.cc should remove these lines:
- #include "tests/cxx/default_tpl_arg_pack_expansion-d1.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg_pack_expansion.cc:
template <typename A, typename B, typename C> struct Triple;
template <typename A, typename B> struct Pair;
template <typename T> struct Trait;  // lines XX-XX+1

***** IWYU_SUMMARY */

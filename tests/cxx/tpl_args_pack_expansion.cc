//===--- tpl_args_pack_expansion.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU doesn't abort on a template specialization written with a
// pack expansion. 'Pair<Ts...>' writes one argument, but the expansion can
// stand for any number of them, so it doesn't mean that 'B' takes a default
// argument. IWYU used to look for a redeclaration giving 'B' a default, and
// aborted when it found none. See default_tpl_arg_pack_expansion.cc for the
// uses IWYU reports in such specializations.

template <typename A, typename B>
struct Pair {};

template <typename T>
struct Trait;

template <typename... Ts>
struct Trait<Pair<Ts...>> {};

/**** IWYU_SUMMARY

(tests/cxx/tpl_args_pack_expansion.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

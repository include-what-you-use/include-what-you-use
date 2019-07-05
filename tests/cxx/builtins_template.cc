//===--- builtins_template.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Built-in templates have no declaration, thus they need no include.

__type_pack_element<0, int> tp;

template <class T, T...> struct A {};
__make_integer_seq<A, int, 5> seq;

class Foo {};
__type_pack_element<0, Foo> foo;

/**** IWYU_SUMMARY

(tests/cxx/builtins_template.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

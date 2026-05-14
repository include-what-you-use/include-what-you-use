//===--- expl_inst_args.cc - iwyu test ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test checks that template arguments are reported from explicit
// instantiation declarations and definitions. For the relationship between
// explicit instantiations and their backing template, see other expl_inst_*
// tests.

// IWYU_ARGS: -I .

#include "tests/cxx/expl_inst_args-d1.h"

// Example covering all relevant entities, based on:
// https://github.com/llvm/llvm-project/issues/191442
template <typename T>
void foo(T x) {}                          // (a) function template
template <typename T> T bar = T{};        // (b) variable template
template <typename T> struct S {          // (c) class template
  void method(T x) {}                     // (d) member function
  static T sval;                          // (e) static data member
  template <typename U> void tmpl(U u) {} // (f) member function template
  struct Inner { T val; };                // (g) nested class
};
template <typename T>
void body() { T x; }                      // (h) function template body
// TODO: These are misattributed from explicit instantiation definitions.
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template <typename T> T S<T>::sval = T{}; // (i) template static member dfn

// Part 1: add explicit instantiation declarations for these template kinds.

// TODO: Language does not need complete Struct1 here.
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
extern template void foo<Struct1>(Struct1);              // (a)
// IWYU: Struct1 needs a declaration
extern template Struct1 bar<Struct1>;                    // (b)
// TODO: Language wants complete Struct1 for nested S::Inner.
// IWYU: Struct1 needs a declaration
extern template struct S<Struct1>;                       // (c)
// TODO: Language does not need complete Struct2 here.
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
extern template void S<Struct2>::method(Struct2);        // (d)
// IWYU: Struct2 needs a declaration
extern template Struct2 S<Struct2>::sval;                // (e) & (i)
// TODO: Language does not need complete Struct2 here.
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
extern template void S<Struct1>::tmpl<Struct2>(Struct2); // (f)
// IWYU: Struct2 needs a declaration
extern template struct S<Struct2>::Inner;                // (g)
// IWYU: Struct1 needs a declaration
extern template void body<Struct1>();                    // (h)

// Part 2: add explicit instantiation definitions for these template kinds.

// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void foo<Struct1>(Struct1);              // (a)
// TODO: Language wants complete Struct1 here for bar dfn.
template Struct1 bar<Struct1>;                    // (b)
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template struct S<Struct1>;                       // (c)
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void S<Struct2>::method(Struct2);        // (d)
// TODO: Language wants complete Struct2 here for sval static member dfn.
// IWYU: Struct2 needs a declaration
template Struct2 S<Struct2>::sval;                // (e) & (i)
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void S<Struct1>::tmpl<Struct2>(Struct2); // (f)
// IWYU: Struct2 needs a declaration
template struct S<Struct2>::Inner;                // (g)
// TODO: Language wants complete Struct1 here for the body instantiation.
// IWYU: Struct1 needs a declaration
template void body<Struct1>();                    // (h)

/**** IWYU_SUMMARY

tests/cxx/expl_inst_args.cc should add these lines:
#include "tests/cxx/expl_inst_args-i1.h"

tests/cxx/expl_inst_args.cc should remove these lines:
- #include "tests/cxx/expl_inst_args-d1.h"  // lines XX-XX

The full include-list for tests/cxx/expl_inst_args.cc:
#include "tests/cxx/expl_inst_args-i1.h"  // for Struct1, Struct2

***** IWYU_SUMMARY */

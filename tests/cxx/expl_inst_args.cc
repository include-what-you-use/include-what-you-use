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
void body1() {
  T x;                                    // (h) function template body
}
template <typename T>
void body2(T*) {
  T x;
}
template <typename Ptr>
void body3() {
  Ptr p;
  (void)sizeof(*p);
}
// TODO: These are misattributed from explicit instantiation definitions.
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template <typename T> T S<T>::sval = T{}; // (i) template static member dfn
template <typename T>
struct S2 {
  void method() {
    T t;                                  // (j) template class method body
  }
};

// IWYU: Struct3 is...*expl_inst_args-i1.h
using Struct3Providing = Struct3;
// IWYU: Struct3 is...*expl_inst_args-i1.h
using Struct3ProvidingPtr = Struct3*;
// IWYU: Struct3 is...*expl_inst_args-i1.h
using S2Struct3Providing = S2<Struct3>;

// Part 1: add explicit instantiation declarations for these template kinds.

// IWYU: Struct1 needs a declaration
extern template void foo<Struct1>(Struct1);              // (a)
// IWYU: Struct1 needs a declaration
extern template Struct1 bar<Struct1>;                    // (b)
// TODO: Language wants complete Struct1 for nested S::Inner.
// IWYU: Struct1 needs a declaration
extern template struct S<Struct1>;                       // (c)
// IWYU: Struct2 needs a declaration
extern template void S<Struct2>::method(Struct2);        // (d)
// IWYU: Struct2 needs a declaration
extern template Struct2 S<Struct2>::sval;                // (e) & (i)
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
extern template void S<Struct1>::tmpl<Struct2>(Struct2); // (f)
// IWYU: Struct2 needs a declaration
extern template struct S<Struct2>::Inner;                // (g)
// IWYU: Struct1 needs a declaration
extern template void body1<Struct1>();                   // (h)
// IWYU: Struct1 needs a declaration
extern template void S2<Struct1>::method();              // (j)

// Part 2: add explicit instantiation definitions for these template kinds.

// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void foo<Struct1>(Struct1);              // (a)
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template Struct1 bar<Struct1>;                    // (b)
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template struct S<Struct1>;                       // (c)
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void S<Struct2>::method(Struct2);        // (d)
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
template Struct2 S<Struct2>::sval;                // (e) & (i)
// IWYU: Struct1 needs a declaration
// IWYU: Struct2 needs a declaration
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void S<Struct1>::tmpl<Struct2>(Struct2); // (f)
// IWYU: Struct2 needs a declaration
template struct S<Struct2>::Inner;                // (g)
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void body1<Struct1>();                   // (h)
// IWYU: Struct2NonProviding is...*expl_inst_args-i2.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void body1<Struct2NonProviding>();
template void body1<Struct3Providing>();
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void body2(Struct1*);
// IWYU: Struct2NonProvidingPtr is...*expl_inst_args-i2.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void body2(Struct2NonProvidingPtr);
template void body2(Struct3ProvidingPtr);
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void body3<Struct1*>();
// IWYU: Struct2NonProvidingPtr is...*expl_inst_args-i2.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void body3<Struct2NonProvidingPtr>();
template void body3<Struct3ProvidingPtr>();
// body4 doesn't provide its default template argument.
// IWYU: body4() is...*expl_inst_args-i2.h
// IWYU: Struct2 is...*expl_inst_args-i1.h
template void body4<>();
// body5 provides its default template argument.
// IWYU: body5() is...*expl_inst_args-i1.h
template void body5<>();
// IWYU: Struct1 needs a declaration
// IWYU: Struct1 is...*expl_inst_args-i1.h
template void S2<Struct1>::method();              // (j)
// IWYU: Struct2 is...*expl_inst_args-i1.h
// IWYU: S2Struct2NonProviding is...*expl_inst_args-i2.h
template void S2Struct2NonProviding::method();
template void S2Struct3Providing::method();

/**** IWYU_SUMMARY

tests/cxx/expl_inst_args.cc should add these lines:
#include "tests/cxx/expl_inst_args-i1.h"
#include "tests/cxx/expl_inst_args-i2.h"

tests/cxx/expl_inst_args.cc should remove these lines:
- #include "tests/cxx/expl_inst_args-d1.h"  // lines XX-XX

The full include-list for tests/cxx/expl_inst_args.cc:
#include "tests/cxx/expl_inst_args-i1.h"  // for Struct1, Struct2, Struct3, body5
#include "tests/cxx/expl_inst_args-i2.h"  // for S2Struct2NonProviding, Struct2NonProviding, Struct2NonProvidingPtr, body4

***** IWYU_SUMMARY */

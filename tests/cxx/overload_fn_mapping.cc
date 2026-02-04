//===--- overload_fn_mapping.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/overload_fn_mapping.imp -I . \
//            -std=c++20

// Tests the ability to distinguish function overloads in symbol mappings.

#include "tests/cxx/overload_fn_mapping-d1.h"

void User() {
  // IWYU: Fn() is...*-i3.h
  Fn();
  // IWYU: Fn(int) is...*-i4.h
  Fn(1);
  // IWYU: Fn(char) is...*-i5.h
  Fn('a');
  // IWYU: Fn(int, int) is...*-i6.h
  Fn(1, 1);
  // IWYU: Fn(int, char) is...*-i7.h
  Fn(1, 'a');
  int i = 0;
  double d = 1.0;
  // IWYU: Fn(int *) is...*-i8.h
  Fn(&i);
  // IWYU: Fn(double &) is...*-i9.h
  Fn(d);
  // IWYU: Fn(unsigned long &&) is...*-i10.h
  Fn(1UL);
  // IWYU: Fn(const double &) is...*-i11.h
  Fn(1.0);
  const int* pi = &i;
  // IWYU: Fn(const int *&) is...*-i12.h
  Fn(pi);
  char* pc = nullptr;
  // IWYU: Fn(char *const &) is...*-i13.h
  Fn(pc);
  // IWYU: Fn() is...*-i3.h
  // IWYU: Fn(int (*)()) is...*-i14.h
  Fn(Fn);
  // IWYU: Fn(double, int *) is...*-i15.h
  Fn(1.0, &i);
  // IWYU: A is...*-i1.h
  // IWYU: Fn(A) is...*-i16.h
  Fn(A{});
  // IWYU: ns::Class is...*-i1.h
  // IWYU: Fn(ns::Class) is...*-i17.h
  Fn(ns::Class{});
  // IWYU: Fn(float, int, ...) is...*-i18.h
  Fn(1.0f, 2, 3);
  // IWYU: Fn(float, int) is...*-i19.h
  Fn(1.0f);
  // IWYU: B is...*-i1.h
  // IWYU: Fn(B) is...*-i20.h
  Fn(B{});
  // IWYU: Fn(long) is...*-i21.h
  Fn(1L);
  unsigned arr[3][5][7];
  // IWYU: Fn(const unsigned int (*)[5][7]) is...*-i22.h
  Fn(arr);
  // Fallback to *-i2.h.
  // IWYU: Fn(char, int) is...*-i2.h
  Fn('a', 1);

  // IWYU: FnTakesAll(...) is...*-i2.h
  FnTakesAll(nullptr);
  // IWYU: FnTakesAll(int) is...*-i3.h
  FnTakesAll(1);

  // Fallback to *-i2.h.
  // IWYU: ns::FnFromNs() is...*-i2.h
  ns::FnFromNs();
  // IWYU: ns::Class is...*-i1.h
  // IWYU: ns::FnFromNs(ns::Class) is...*-i3.h
  ns::FnFromNs(ns::Class{});
  // IWYU: ns::C is...*-i1.h
  ns::C c;
  // IWYU: ns::FnFromNs(const ns::C &) is...*-i4.h
  FnFromNs(c);
  // IWYU: ns::FnFromNs(const ns::C &) is...*-i4.h
  ns::FnFromNs(c);
  // IWYU: ns::FnFromNs(const ns::C &) is...*-i4.h
  ns::inl::FnFromNs(c);

  // IWYU: A is...*-i1.h
  A a1, a2;
  // IWYU: B is...*-i1.h
  B b1, b2;
  // TODO: complete A type is not required here.
  // IWYU: A is...*-i1.h
  // IWYU: operator==(const A &, const A &) is...*-i2.h
  (void)(a1 == a2);
  // TODO: complete B type is not required here.
  // IWYU: B is...*-i1.h
  // IWYU: operator==(const B &, const B &) is...*-i3.h
  (void)(b1 == b2);
  // IWYU: A is...*-i1.h
  // IWYU: B is...*-i1.h
  // IWYU: operator==(A, B) is...*-i4.h
  (void)(a1 == b1);
  // Fallback mapping is used here.
  // IWYU: A is...*-i1.h
  // IWYU: B is...*-i1.h
  // IWYU: operator==(B, A) is...*-i5.h
  (void)(b1 == a1);

  // IWYU: TplFn(:0, :1) is...*-i2.h
  TplFn(1, 2);
  // IWYU: TplFn(:1, :0 *) is...*-i3.h
  TplFn(1, &i);
  // IWYU: Tpl is...*-i1.h
  // IWYU: TplFn(Tpl<:0>) is...*-i4.h
  TplFn(Tpl<int>{});
  int arr2[7];
  // IWYU: TplFn(:0 (&)[:1]) is...*-i5.h
  TplFn(arr2);
}

/**** IWYU_SUMMARY

tests/cxx/overload_fn_mapping.cc should add these lines:
#include "tests/cxx/overload_fn_mapping-i1.h"
#include "tests/cxx/overload_fn_mapping-i10.h"
#include "tests/cxx/overload_fn_mapping-i11.h"
#include "tests/cxx/overload_fn_mapping-i12.h"
#include "tests/cxx/overload_fn_mapping-i13.h"
#include "tests/cxx/overload_fn_mapping-i14.h"
#include "tests/cxx/overload_fn_mapping-i15.h"
#include "tests/cxx/overload_fn_mapping-i16.h"
#include "tests/cxx/overload_fn_mapping-i17.h"
#include "tests/cxx/overload_fn_mapping-i18.h"
#include "tests/cxx/overload_fn_mapping-i19.h"
#include "tests/cxx/overload_fn_mapping-i2.h"
#include "tests/cxx/overload_fn_mapping-i20.h"
#include "tests/cxx/overload_fn_mapping-i21.h"
#include "tests/cxx/overload_fn_mapping-i22.h"
#include "tests/cxx/overload_fn_mapping-i3.h"
#include "tests/cxx/overload_fn_mapping-i4.h"
#include "tests/cxx/overload_fn_mapping-i5.h"
#include "tests/cxx/overload_fn_mapping-i6.h"
#include "tests/cxx/overload_fn_mapping-i7.h"
#include "tests/cxx/overload_fn_mapping-i8.h"
#include "tests/cxx/overload_fn_mapping-i9.h"

tests/cxx/overload_fn_mapping.cc should remove these lines:
- #include "tests/cxx/overload_fn_mapping-d1.h"  // lines XX-XX

The full include-list for tests/cxx/overload_fn_mapping.cc:
#include "tests/cxx/overload_fn_mapping-i1.h"  // for A, B, C, Class, Tpl
#include "tests/cxx/overload_fn_mapping-i10.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i11.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i12.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i13.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i14.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i15.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i16.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i17.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i18.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i19.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i2.h"  // for Fn, FnFromNs, FnTakesAll, TplFn, operator==
#include "tests/cxx/overload_fn_mapping-i20.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i21.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i22.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i3.h"  // for Fn, FnFromNs, FnTakesAll, TplFn, operator==
#include "tests/cxx/overload_fn_mapping-i4.h"  // for Fn, FnFromNs, TplFn, operator==
#include "tests/cxx/overload_fn_mapping-i5.h"  // for Fn, TplFn, operator==
#include "tests/cxx/overload_fn_mapping-i6.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i7.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i8.h"  // for Fn
#include "tests/cxx/overload_fn_mapping-i9.h"  // for Fn

***** IWYU_SUMMARY */

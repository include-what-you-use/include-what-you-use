//===--- iwyu_stricter_than_cpp-autocast.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef IWYU_STRICTER_THAN_CPP_AUTOCAST_H_
#define IWYU_STRICTER_THAN_CPP_AUTOCAST_H_

// The two rules the author has to follow to disable iwyu's
// stricter-than-C++ rule and force it to fall back on the c++
// requirement (forward-declare ok):
// (1) forward-declare the relevant type
// (2) do not directly #include the definition of the relevant type.

#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"

// --- Autocast types.

struct DirectStruct1;
struct IndirectStruct2;

void FnValues(
    // Requires the full type because it does not obey rule (1)
    // IWYU: IndirectStruct1 needs a declaration
    // IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    IndirectStruct1 ic1,
    // This also does not obey rule (1): it's -d1 that does the fwd-declaring.
    // IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    struct IndirectStructForwardDeclaredInD1 icfdid1,
    // Requires the full type because it does not obey rule (2)
    DirectStruct1 dc1,
    // Requires the full type because it does not obey rules (1) *or* (2)
    struct DirectStruct2 dc2,
    // Does not require full type because it obeys all the rules.
    IndirectStruct2 ic2);

void FnRefs(
    // IWYU: IndirectStruct1 needs a declaration
    // IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    const IndirectStruct1& ic1,
    // IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    const struct IndirectStructForwardDeclaredInD1& icfdid1,
    const DirectStruct3& dc1, const struct DirectStruct4& dc2,
    const IndirectStruct2& ic2);

// --- Now do it all again, with templates!

template <typename T>
struct TplDirectStruct1;
template <typename T>
struct TplIndirectStruct2;

void TplFnValues(
    // IWYU: TplIndirectStruct1 needs a declaration
    // IWYU: TplIndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    TplIndirectStruct1<char> ic1,
    // A bit of an asymmetry with the non-tpl case: 'struct
    // IndirectStructForwardDeclaredInD1' does not need to be
    // forward-declared because it's elaborated, but template types
    // need to be forward-declared even when they're elaborated.  (Of
    // course, the fwd-decl requirement will be superceded by the
    // full-type requirement due to autocast, but we report both.)
    // IWYU: TplIndirectStructForwardDeclaredInD1 needs a declaration
    // IWYU: TplIndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    struct TplIndirectStructForwardDeclaredInD1<char> icfdid1,
    TplDirectStruct1<char> dc1, struct TplDirectStruct2<char> dc2,
    TplIndirectStruct2<char> ic2);

void TplFnRefs(
    // IWYU: TplIndirectStruct1 needs a declaration
    // IWYU: TplIndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    const TplIndirectStruct1<char>& ic1,
    // IWYU: TplIndirectStructForwardDeclaredInD1 needs a declaration
    // IWYU: TplIndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h.*for autocast
    const struct TplIndirectStructForwardDeclaredInD1<char>& icfdid1,
    const TplDirectStruct3<char>& dc1, const struct TplDirectStruct4<char>& dc2,
    const TplIndirectStruct2<char>& ic2);

// --- The rules do not apply for friend functions declarations.

struct AutocastStruct {
  // IWYU: IndirectStruct1 needs a declaration
  friend void ClassFn1(const IndirectStruct1&);
  // IWYU: TplIndirectStruct1 needs a declaration
  friend void ClassFn2(TplIndirectStruct1<char>);
};

#endif  // IWYU_STRICTER_THAN_CPP_AUTOCAST_H_

/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp-autocast.h should add these lines:
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"

tests/cxx/iwyu_stricter_than_cpp-autocast.h should remove these lines:
- struct DirectStruct1;  // lines XX-XX
- template <typename T> struct TplDirectStruct1;  // lines XX-XX+1

The full include-list for tests/cxx/iwyu_stricter_than_cpp-autocast.h:
#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"  // for DirectStruct1, DirectStruct2, DirectStruct3, DirectStruct4, TplDirectStruct1, TplDirectStruct2, TplDirectStruct3, TplDirectStruct4
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"  // for IndirectStruct1, IndirectStructForwardDeclaredInD1, TplIndirectStruct1, TplIndirectStructForwardDeclaredInD1
struct IndirectStruct2;  // lines XX-XX
template <typename T> struct TplIndirectStruct2;  // lines XX-XX+1

***** IWYU_SUMMARY */

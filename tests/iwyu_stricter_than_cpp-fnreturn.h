//===--- iwyu_stricter_than_cpp-fnreturn.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The two rules the author has to follow to disable iwyu's
// stricter-than-C++ rule and force it to fall back on the c++
// requirement (forward-declare ok):
// (1) forward-declare the relevant type
// (2) do not directly #include the definition of the relevant type.

#include "tests/iwyu_stricter_than_cpp-d1.h"

// --- Return values of functions.

// Requires the full type because it does not obey rule (1)
// IWYU: IndirectStruct1 needs a declaration
// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
IndirectStruct1 DoesNotForwardDeclareFn();

// This also does not obey rule (1): it's -d1 that does the fwd-declaring.
// IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
struct IndirectStructForwardDeclaredInD1 DoesNotForwardDeclareProperlyFn();

// Requires the full type because it does not obey rule (2)
struct DirectStruct1;
DirectStruct1 IncludesFn();

// Requires the full type because it does not obey rules (1) *or* (2)
DirectStruct2 DoesNotForwardDeclareAndIncludesFn();

// Does not require full type because it obeys all the rules.
struct IndirectStruct2;
IndirectStruct2 DoesEverythingRightFn();

// --- Now do it all again, with templates!

// IWYU: TplIndirectStruct1 needs a declaration
// IWYU: TplIndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
TplIndirectStruct1<int> TplDoesNotForwardDeclareFn();

// A bit of an asymmetry with the non-tpl case: 'struct
// IndirectStructForwardDeclaredInD1' does not need to be
// forward-declared because it's elaborated, but template types need
// to be forward-declared even when they're elaborated.
// IWYU: TplIndirectStructForwardDeclaredInD1 needs a declaration
struct TplIndirectStructForwardDeclaredInD1<int>
// IWYU: TplIndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
TplDoesNotForwardDeclareProperlyFn();

template <typename T> struct TplDirectStruct1;
TplDirectStruct1<int> TplIncludesFn();

TplDirectStruct2<int> TplDoesNotForwardDeclareAndIncludesFn();

template <typename T> struct TplIndirectStruct2;
TplIndirectStruct2<int> TplDoesEverythingRightFn();

// Another way to forward-declare a class template.
template <> struct TplIndirectStruct2<float>;
TplIndirectStruct2<float> TplDoesEverythingRightAgainFn();

// --- The rules do not apply for friend functions.

struct FnreturnStruct {
  // IWYU: IndirectStruct1 needs a declaration
  friend const IndirectStruct1& ClassFn1();
  // IWYU: TplIndirectStruct1 needs a declaration
  friend TplIndirectStruct1<char> ClassFn2();
};



/**** IWYU_SUMMARY

tests/iwyu_stricter_than_cpp-fnreturn.h should add these lines:
#include "tests/iwyu_stricter_than_cpp-i1.h"

tests/iwyu_stricter_than_cpp-fnreturn.h should remove these lines:
- struct DirectStruct1;  // lines XX-XX
- template <typename T> struct TplDirectStruct1;  // lines XX-XX

The full include-list for tests/iwyu_stricter_than_cpp-fnreturn.h:
#include "tests/iwyu_stricter_than_cpp-d1.h"  // for DirectStruct1, DirectStruct2, TplDirectStruct1, TplDirectStruct2
#include "tests/iwyu_stricter_than_cpp-i1.h"  // for IndirectStruct1, IndirectStructForwardDeclaredInD1, TplIndirectStruct1, TplIndirectStructForwardDeclaredInD1
struct IndirectStruct2;  // lines XX-XX
template <typename T> struct TplIndirectStruct2;  // lines XX-XX

***** IWYU_SUMMARY */

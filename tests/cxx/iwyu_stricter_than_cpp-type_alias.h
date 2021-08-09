//===--- iwyu_stricter_than_cpp-type_alias.h - test input file for iwyu ---===//
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

#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"

// --- Type aliases.

// Requires the full type because it does not obey rule (1)
// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
using DoesNotForwardDeclareAl = IndirectStruct1;

// This also does not obey rule (1): it's -d1 that does the fwd-declaring.
// IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
using DoesNotForwardDeclareProperlyAl = IndirectStructForwardDeclaredInD1;

// Requires the full type because it does not obey rule (2)
struct DirectStruct1;
using IncludesAl = DirectStruct1;

// Requires the full type because it does not obey rules (1) *or* (2)
using DoesNotForwardDeclareAndIncludesAl = DirectStruct2;

// Does not require full type because it obeys all the rules.
struct IndirectStruct2;
using DoesEverythingRightAl = IndirectStruct2;

// --- Now do it all again, with templates!

// IWYU: TplIndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
using TplDoesNotForwardDeclareAl = TplIndirectStruct1<int>;

using TplDoesNotForwardDeclareProperlyAl
// IWYU: TplIndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
    = TplIndirectStructForwardDeclaredInD1<int>;

template <typename T> struct TplDirectStruct1;
using TplIncludesAl = TplDirectStruct1<int>;

using TplDoesNotForwardDeclareAndIncludesAl = TplDirectStruct2<int>;

template <typename T> struct TplIndirectStruct2;
using TplDoesEverythingRightAl = TplIndirectStruct2<int>;

// Another way to forward-declare a class template.
template <> struct TplIndirectStruct2<float>;
using TplDoesEverythingRightAgainAl = TplIndirectStruct2<float>;


/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp-type_alias.h should add these lines:
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"

tests/cxx/iwyu_stricter_than_cpp-type_alias.h should remove these lines:
- struct DirectStruct1;  // lines XX-XX
- template <typename T> struct TplDirectStruct1;  // lines XX-XX

The full include-list for tests/cxx/iwyu_stricter_than_cpp-type_alias.h:
#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"  // for DirectStruct1, DirectStruct2, TplDirectStruct1, TplDirectStruct2
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"  // for IndirectStruct1, IndirectStructForwardDeclaredInD1, TplIndirectStruct1, TplIndirectStructForwardDeclaredInD1
struct IndirectStruct2;  // lines XX-XX
template <typename T> struct TplIndirectStruct2;  // lines XX-XX

***** IWYU_SUMMARY */

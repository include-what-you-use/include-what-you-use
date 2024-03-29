//===--- iwyu_stricter_than_cpp-typedefs.h - test input file for iwyu -----===//
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
#include "tests/cxx/iwyu_stricter_than_cpp-d4.h"

// --- Typedefs.

// Requires the full type because it does not obey rule (1)
// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
typedef IndirectStruct1 DoesNotForwardDeclare;

// This also does not obey rule (1): it's -d1 that does the fwd-declaring.
// IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
typedef IndirectStructForwardDeclaredInD1 DoesNotForwardDeclareProperly;

// Requires the full type because it does not obey rule (2)
struct DirectStruct1;
typedef DirectStruct1 Includes;

// Essentially the same; should require corresponding header inclusion here.
typedef struct DirectStruct3 IncludesElaborated;

// The same: the public header for MappedToD1 is directly included.
struct MappedToD1;
typedef MappedToD1 IncludesPublicHeader;

// Requires the full type because it does not obey rules (1) *or* (2)
typedef DirectStruct2 DoesNotForwardDeclareAndIncludes;

// Does not require full type because it obeys all the rules.
struct IndirectStruct2;
typedef IndirectStruct2 DoesEverythingRight;

// --- Now do it all again, with templates!

// IWYU: IndirectStruct1 needs a declaration
// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
typedef TplIndirectStruct1<int> TplDoesNotForwardDeclare;

// IWYU: IndirectStructForwardDeclaredInD1 needs a declaration
// IWYU: IndirectStructForwardDeclaredInD1 is...*iwyu_stricter_than_cpp-i1.h
typedef TplIndirectStructForwardDeclaredInD1<int>
TplDoesNotForwardDeclareProperly;

template <typename T> struct TplDirectStruct1;
typedef TplDirectStruct1<int> TplIncludes;

typedef TplDirectStruct2<int> TplDoesNotForwardDeclareAndIncludes;

template <typename T> struct TplIndirectStruct2;
typedef TplIndirectStruct2<int> TplDoesEverythingRight;

// Another way to forward-declare a class template.
template <> struct TplIndirectStruct2<float>;
typedef TplIndirectStruct2<float> TplDoesEverythingRightAgain;

// --- With user-defined types as template parameters.

// struct IndirectStruct2; (fwd-declared above)
template <typename TFullTypeUsed, typename TForwardDeclarable>
struct TplIndirectStruct3;

// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
typedef TplIndirectStruct3<IndirectStruct1, IndirectStruct2>
    TplOnlyArgumentTypeProvided;

typedef TplIndirectStruct3<IndirectStruct2, IndirectStruct2>
    TplAllForwardDeclared;

// IWYU: IndirectStruct1 is...*iwyu_stricter_than_cpp-i1.h
typedef TplDirectStruct7<IndirectStruct1, IndirectStruct2>
    TplAllNeededTypesProvided;

typedef TplDirectStruct7<IndirectStruct2, IndirectStruct2>
    TplOnlyTemplateProvided;

// --- Special typedefs, for nested name testing

struct IndirectStruct3;
typedef IndirectStruct3 IndirectStruct3NonProvidingTypedef;

struct IndirectStruct4;
typedef IndirectStruct4 IndirectStruct4NonProvidingTypedef;


/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp-typedefs.h should add these lines:
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"

tests/cxx/iwyu_stricter_than_cpp-typedefs.h should remove these lines:
- struct DirectStruct1;  // lines XX-XX
- struct MappedToD1;  // lines XX-XX
- template <typename T> struct TplDirectStruct1;  // lines XX-XX

The full include-list for tests/cxx/iwyu_stricter_than_cpp-typedefs.h:
#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"  // for DirectStruct1, DirectStruct2, DirectStruct3, MappedToD1, TplDirectStruct1, TplDirectStruct2
#include "tests/cxx/iwyu_stricter_than_cpp-d4.h"  // for TplDirectStruct7
#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"  // for IndirectStruct1, IndirectStructForwardDeclaredInD1, TplIndirectStruct1, TplIndirectStructForwardDeclaredInD1
struct IndirectStruct2;  // lines XX-XX
struct IndirectStruct3;  // lines XX-XX
struct IndirectStruct4;  // lines XX-XX
template <typename T> struct TplIndirectStruct2;  // lines XX-XX
template <typename TFullTypeUsed, typename TForwardDeclarable> struct TplIndirectStruct3;  // lines XX-XX+1

***** IWYU_SUMMARY */

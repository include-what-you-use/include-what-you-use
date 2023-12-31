//===--- iwyu_stricter_than_cpp-def_tpl_arg.h - test input file for iwyu --===//
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
#include "tests/cxx/iwyu_stricter_than_cpp-d3.h"

struct DirectStruct1;
struct IndirectStruct2;

// --- Default type template arguments.

template <
    // Requires the full type because it does not obey rule (1)
    // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
    typename DoesNotForwardDeclare = IndirectStruct3,
    // Requires the full type because it does not obey rule (2)
    typename Includes = DirectStruct1,
    // Requires the full type because it does not obey rules (1) *or* (2)
    typename DoesNotForwardDeclareAndIncludes = DirectStruct2,
    // Does not require full type because it obeys all the rules.
    typename DoesEverythingRight = IndirectStruct2>
struct TplWithDefaultArgs {
  TplWithDefaultArgs();

  DoesNotForwardDeclare a;
  Includes b;
  DoesNotForwardDeclareAndIncludes c;
  DoesEverythingRight d;
};

/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp-def_tpl_arg.h should add these lines:
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"

tests/cxx/iwyu_stricter_than_cpp-def_tpl_arg.h should remove these lines:
- #include "tests/cxx/iwyu_stricter_than_cpp-d3.h"  // lines XX-XX
- struct DirectStruct1;  // lines XX-XX

The full include-list for tests/cxx/iwyu_stricter_than_cpp-def_tpl_arg.h:
#include "tests/cxx/iwyu_stricter_than_cpp-d1.h"  // for DirectStruct1, DirectStruct2
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"  // for IndirectStruct3
struct IndirectStruct2;  // lines XX-XX

***** IWYU_SUMMARY */

//===--- macro_location.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "tests/cxx/indirect.h"
#include "tests/cxx/macro_location-d2.h"
#include "tests/cxx/macro_location-d3.h"
#include "tests/cxx/macro_location-d4.h"

class Foo;
static Foo *foo = 0;

class HClass { };

// IWYU: Foo is...*macro_location-i3.h
const int s = ARRAYSIZE(foo);
// Should not need a declaration of NewClass_Bar, or NewClass.
NEW_CLASS(Bar);
// This shouldn't cause weird iwyu issues between us and macro_location-d2.h.
USE_CLASS(HClass);
// This shouldn't cause macro_location-d1.h to need to include us for HClass.
CREATE_VAR(HClass);
// This should force us to #include a definition of IndirectClass, because it's
// not included by DECLARE_INDIRECT's file.
DECLARE_INDIRECT(global_indirect);
// This case with Foo is similar.
// IWYU: Foo is...*macro_location-i3.h
auto foo_size = SIZEOF_FOO_NON_PROVIDING;
// '-d2.h' provides a forward-declaration of Bar so it should not be reported
// here.
DECLARE_BAR_PTR(global_bar_ptr);

// Macro-concatenated locations end up in <scratch space>, check that they're
// properly attributed to expansion.
#define CONCAT(a, b) a##b

// IWYU: ConcatFwdDeclClass needs a declaration
CONCAT(Concat, FwdDeclClass) *global_concat_ptr;

// IWYU: ConcatClass is...*macro_location-i4.h
CONCAT(Concat, Class) global_concat;

// Make sure types defined and used only within a macro definition file
// aren't attributed to the macro expansion loc.

// IWYU: UNNAMED_TYPE_IN_MACRO is...*macro_location-i5.h
UNNAMED_TYPE_IN_MACRO(500);

void func() {
  LOG_INFO("hello");
  DECLARE_AND_USE_CLASS("hello again");
  // Although 'ns' token is inside the macro definition, IWYU should report
  // the called function here because it is not provided by the macro.
  // IWYU: ns::ConcatFn1() is...*macro_location-i4.h
  CALL_NS_CONCAT_FN(1)
  // A macro in '-d2.h' replaces OrigName with ShadowName. The function
  // declaration from '-i3.h' should still be reported.
  // IWYU: ShadowName() is...*macro_location-i3.h
  OrigName();
  // IWYU: MacroDeclaredFn() is...*macro_location-i2.h
  CALL_MACRO_DECLARED_FN;
  CALL_FN_WITH_REDECL;
  // IWYU should not report '-i1.h' for FnDeclaredInPublicHeader because it is
  // provided by the public header of the macro ('-i2.h').
  // IWYU: CALL_FN_DECLARED_IN_PUBLIC_HEADER is...*macro_location-i2.h
  CALL_FN_DECLARED_IN_PUBLIC_HEADER;
  // IWYU should not report ProvidedFnDeclaredAfterMacro because it is provided
  // by the header defining the macro.
  // IWYU: CALL_FN_DECLARED_AFTER_MACRO is...*macro_location-i2.h
  CALL_FN_DECLARED_AFTER_MACRO;
  // IWYU should report neither INTERNAL_MACRO nor InternalFnDeclaredAfterMacro.
  USE_INTERNAL_MACRO;
}

#define ANIMALS Cat, Lion, Tiger

struct AnimalArrays {
  enum Animal : unsigned { ANIMALS, Count };

  constexpr static Animal array1[Count] = {ANIMALS};
  static Animal array2[Count];
};

/**** IWYU_SUMMARY

tests/cxx/macro_location.h should add these lines:
#include "tests/cxx/macro_location-i2.h"
#include "tests/cxx/macro_location-i3.h"
#include "tests/cxx/macro_location-i4.h"
#include "tests/cxx/macro_location-i5.h"

tests/cxx/macro_location.h should remove these lines:
- #include "tests/cxx/macro_location-d3.h"  // lines XX-XX
- class Foo;  // lines XX-XX

The full include-list for tests/cxx/macro_location.h:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/macro_location-d2.h"  // for ARRAYSIZE, CALL_FN_WITH_REDECL, CALL_MACRO_DECLARED_FN, CALL_NS_CONCAT_FN, CREATE_VAR, DECLARE_BAR_PTR, DECLARE_INDIRECT, NEW_CLASS, OrigName, SIZEOF_FOO_NON_PROVIDING, USE_CLASS, USE_INTERNAL_MACRO
#include "tests/cxx/macro_location-d4.h"  // for DECLARE_AND_USE_CLASS, LOG_INFO
#include "tests/cxx/macro_location-i2.h"  // for CALL_FN_DECLARED_AFTER_MACRO, CALL_FN_DECLARED_IN_PUBLIC_HEADER, MacroDeclaredFn
#include "tests/cxx/macro_location-i3.h"  // for Foo, ShadowName
#include "tests/cxx/macro_location-i4.h"  // for ConcatClass, ConcatFn1, ConcatFwdDeclClass (ptr only)
#include "tests/cxx/macro_location-i5.h"  // for UNNAMED_TYPE_IN_MACRO


***** IWYU_SUMMARY */

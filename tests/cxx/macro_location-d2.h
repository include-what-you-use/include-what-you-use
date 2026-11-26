//===--- macro_location-d2.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location-d1.h"
#include "tests/cxx/macro_location-i1.h"

class IndirectClass;

#define DECLARE_INDIRECT(name) IndirectClass name;

// This header doesn't include '-i3.h' defining Foo, so it should be reported
// at the use location. IWYU should not suggest including '-i3.h' here.
#define SIZEOF_FOO_NON_PROVIDING sizeof(Foo)

class Bar;

#define DECLARE_BAR_PTR(name) Bar* name;

#define ARRAYSIZE(x)  ( sizeof(x) / sizeof(*(x)) )

#define NEW_CLASS(name)                         \
  class NewClass_##name : public NewClass {     \
   public:                                      \
    OtherClass o;                               \
  };

// This macro is tricky because myclass_##classname involves a type
// that's defined in scratch space.  Make sure this doesn't result in
// an IWYU violation.  Nor should classname used *not* in a macro
// concatenation (as the return value of Init).
#define USE_CLASS(classname)                    \
  struct Use_##classname {                      \
    Use_##classname() { Init(); }               \
    classname* Init() { return 0; }             \
  };                                            \
  static Use_##classname myclass_##classname

#define CREATE_VAR(typ)   typ create_var

#define CALL_NS_CONCAT_FN(num) \
  {                            \
    ns::ConcatFn##num();       \
  }

#define OrigName ShadowName

// IWYU should report ImplicitConvStruct neither here...
#define ONE 1

int GetInt();
// ... nor here.
#define GET_INT GetInt()

// This macro doesn't provide MacroDeclaredFn although the spelling location
// of its declaration ('-i1.h') is directly included.
#define CALL_MACRO_DECLARED_FN MacroDeclaredFn()

void FnWithRedecl();

// This macro should be considered as providing though there is another
// declaration of FnWithRedecl not included here.
#define CALL_FN_WITH_REDECL FnWithRedecl()

#define USE_INTERNAL_MACRO INTERNAL_MACRO

/**** IWYU_SUMMARY

tests/cxx/macro_location-d2.h should add these lines:

tests/cxx/macro_location-d2.h should remove these lines:
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/macro_location-d2.h:
#include "tests/cxx/macro_location-d1.h"  // for NewClass, OtherClass
#include "tests/cxx/macro_location-i1.h"  // for INTERNAL_MACRO
class Bar;  // lines XX-XX

***** IWYU_SUMMARY */

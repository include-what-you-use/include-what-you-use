//===--- macro_location-d2.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location-d1.h"

// The forward-declare should get removed as the use of the macro
// is attributed to the users of DECLARE_INDIRECT macro, not this file.
class IndirectClass;

#define DECLARE_INDIRECT(name) IndirectClass name;

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


/**** IWYU_SUMMARY

tests/cxx/macro_location-d2.h should add these lines:

tests/cxx/macro_location-d2.h should remove these lines:
- #include "tests/cxx/macro_location-d1.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/macro_location-d2.h:

***** IWYU_SUMMARY */

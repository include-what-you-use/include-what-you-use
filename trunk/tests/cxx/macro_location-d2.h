//===--- macro_location-d2.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location-d1.h"

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

(tests/cxx/macro_location-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

//===--- macro_location-d2.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/macro_location-d1.h"

#define ARRAYSIZE(x)  ( sizeof(x) / sizeof(*(x)) )

#define NEW_CLASS(name)                         \
  class NewClass_##name : public NewClass {     \
   public:                                      \
    OtherClass o;                               \
  };


/**** IWYU_SUMMARY

(tests/macro_location-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

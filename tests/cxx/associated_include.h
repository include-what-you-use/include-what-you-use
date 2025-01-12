//===--- associated_include.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"
#include "tests/cxx/associated_include-i1.h"
#include "tests/cxx/associated_include-i2.h"

namespace hfile {
AssociatedIncludeClass aic;
// IWYU: ClassFromI3 is...*associated_include-i3.h
ClassFromI3 cfi3;
}

/**** IWYU_SUMMARY

tests/cxx/associated_include.h should add these lines:
#include "tests/cxx/associated_include-i3.h"

tests/cxx/associated_include.h should remove these lines:
- #include "tests/cxx/associated_include-i2.h"  // lines XX-XX
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/associated_include.h:
#include "tests/cxx/associated_include-i1.h"  // for AssociatedIncludeClass
#include "tests/cxx/associated_include-i3.h"  // for ClassFromI3

***** IWYU_SUMMARY */

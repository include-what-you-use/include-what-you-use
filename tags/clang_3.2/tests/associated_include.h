//===--- associated_include.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/indirect.h"
#include "tests/associated_include-i1.h"

namespace hfile {
AssociatedIncludeClass aic;
}

/**** IWYU_SUMMARY

tests/associated_include.h should add these lines:

tests/associated_include.h should remove these lines:
- #include "tests/indirect.h"  // lines XX-XX

The full include-list for tests/associated_include.h:
#include "tests/associated_include-i1.h"  // for AssociatedIncludeClass

***** IWYU_SUMMARY */

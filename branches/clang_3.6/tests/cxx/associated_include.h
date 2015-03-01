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

namespace hfile {
AssociatedIncludeClass aic;
}

/**** IWYU_SUMMARY

tests/cxx/associated_include.h should add these lines:

tests/cxx/associated_include.h should remove these lines:
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/associated_include.h:
#include "tests/cxx/associated_include-i1.h"  // for AssociatedIncludeClass

***** IWYU_SUMMARY */

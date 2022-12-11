//===--- pragma_export_fwd.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Forward declarations exported from this header are not used here, so it will
// be suggested for removal.
#include "tests/cxx/pragma_export_fwd-d1.h"

// Unused forward declarations in associated header would normally be moved to
// main source file. Make sure they are left alone when exported.
class FwdDecl1;  // IWYU pragma: export

// IWYU pragma: begin_exports
class FwdDecl2;
// IWYU pragma: end_exports

/**** IWYU_SUMMARY

tests/cxx/pragma_export_fwd.h should add these lines:

tests/cxx/pragma_export_fwd.h should remove these lines:
- #include "tests/cxx/pragma_export_fwd-d1.h"  // lines XX-XX

The full include-list for tests/cxx/pragma_export_fwd.h:
class FwdDecl1;  // lines XX-XX
class FwdDecl2;  // lines XX-XX

***** IWYU_SUMMARY */

//===--- pragma_export_fwd-d2.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/pragma_export_fwd-i1.h"

class FwdDecl5;  // IWYU pragma: export

extern FwdDecl5* pfd5;

class FwdDecl6;  // IWYU pragma: export

// There is no need in '-i1.h' here because this header already has its own
// forward-declaration of FwdDecl6. IWYU should suggest removing the #include.
extern FwdDecl6* pfd6;

/**** IWYU_SUMMARY

tests/cxx/pragma_export_fwd-d2.h should add these lines:

tests/cxx/pragma_export_fwd-d2.h should remove these lines:
- #include "tests/cxx/pragma_export_fwd-i1.h"  // lines XX-XX

The full include-list for tests/cxx/pragma_export_fwd-d2.h:
class FwdDecl5;  // lines XX-XX
class FwdDecl6;  // lines XX-XX

***** IWYU_SUMMARY */

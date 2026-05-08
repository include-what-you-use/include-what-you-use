//===--- pragma_export_fwd.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/pragma_export_fwd.h"

// Uses do not trigger warnings as they are already provided for.
void a(const FwdDecl1&);
void b(const FwdDecl2*);
// IWYU: FwdDecl3 needs a declaration
void c(const FwdDecl3&);
// IWYU: FwdDecl4 needs a declaration
void d(const FwdDecl4*);

// IWYU should not suggest '-i2.h' for the forward-declaration because
// the definition should be available.
// IWYU: FullAndFwdDeclUse needs a declaration
// IWYU: FullAndFwdDeclUse is...*-i1.h
FullAndFwdDeclUse full_use, *fwd_decl_use;

/**** IWYU_SUMMARY

tests/cxx/pragma_export_fwd.cc should add these lines:
#include "tests/cxx/pragma_export_fwd-d1.h"
#include "tests/cxx/pragma_export_fwd-i1.h"

tests/cxx/pragma_export_fwd.cc should remove these lines:

The full include-list for tests/cxx/pragma_export_fwd.cc:
#include "tests/cxx/pragma_export_fwd.h"
#include "tests/cxx/pragma_export_fwd-d1.h"  // for FwdDecl3 (ptr only), FwdDecl4 (ptr only)
#include "tests/cxx/pragma_export_fwd-i1.h"  // for FullAndFwdDeclUse

***** IWYU_SUMMARY */

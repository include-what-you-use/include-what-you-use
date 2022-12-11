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
void c(const FwdDecl3&);
void d(const FwdDecl4*);

/**** IWYU_SUMMARY

tests/cxx/pragma_export_fwd.cc should add these lines:
#include "tests/cxx/pragma_export_fwd-d1.h"

tests/cxx/pragma_export_fwd.cc should remove these lines:

The full include-list for tests/cxx/pragma_export_fwd.cc:
#include "tests/cxx/pragma_export_fwd.h"
#include "tests/cxx/pragma_export_fwd-d1.h"  // for FwdDecl3, FwdDecl4

***** IWYU_SUMMARY */

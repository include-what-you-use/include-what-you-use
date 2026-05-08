//===--- pragma_export_fwd_path_local.cc - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "pragma_export_fwd_path_local-d1.h"

// IWYU: FwdDecl needs a declaration
FwdDecl* pFwdDecl;

/**** IWYU_SUMMARY

tests/cxx/pragma_export_fwd_path_local.cc should add these lines:
#include "pragma_export_fwd_path_local-i1.h"

tests/cxx/pragma_export_fwd_path_local.cc should remove these lines:
- #include "pragma_export_fwd_path_local-d1.h"  // lines XX-XX

The full include-list for tests/cxx/pragma_export_fwd_path_local.cc:
#include "pragma_export_fwd_path_local-i1.h"  // for FwdDecl (ptr only)

***** IWYU_SUMMARY */

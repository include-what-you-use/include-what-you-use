//===--- pragma_export_fwd-d1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/pragma_export_fwd-i1.h"
#include "tests/cxx/pragma_export_fwd-i2.h"

class FwdDecl3;  // IWYU pragma: export

// IWYU pragma: begin_exports
class FwdDecl4;
// IWYU pragma: end_exports

//===--- pragma_export_fwd-i1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRAGMA_EXPORT_FWD_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRAGMA_EXPORT_FWD_I1_H_

class FullAndFwdDeclUse {};

class FwdDecl6;  // IWYU pragma: export

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRAGMA_EXPORT_FWD_I1_H_

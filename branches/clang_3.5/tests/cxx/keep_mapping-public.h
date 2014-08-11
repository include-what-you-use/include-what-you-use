//===--- keep_mapping-public.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Makes sure that when we #include a file that is part of an explicit
// mapping in iwyu_include_picker.cc, that iwyu doesn't say to remove
// that include.

// This is part of a mapping in iwyu_include_picker.cc.
#include "tests/cxx/keep_mapping-priv.h"
// This is part of a glob-mapping in iwyu_include_picker.cc
#include "tests/cxx/keep_mapping-private.h"
// This has an explicit pragma
#include "tests/cxx/keep_mapping-pragma1.h"  // IWYU pragma: export
// This is also an explicit pragma
// IWYU pragma: begin_exports
#include "tests/cxx/keep_mapping-pragma2.h"
// IWYU pragma: end_exports

const int kInt = 5;

/**** IWYU_SUMMARY

(tests/cxx/keep_mapping-public.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

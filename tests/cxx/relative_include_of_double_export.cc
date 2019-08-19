//===--- relative_include_of_double_export.cc - test input file for iwyu --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The purpose of this test is to ensure that the following relative include
// (i.e. using "" to include something at a location relative to this file)
// remains a relative include rather than being replaced by a different path.
#include "relative_include_of_double_export-d1.h"

// This class is defined two layers deep within an export double header
// included via the above.
PrivateClass x;

/**** IWYU_SUMMARY

(tests/cxx/relative_include_of_double_export.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

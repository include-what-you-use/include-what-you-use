//===--- mapping_to_self.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/mapping_to_self.imp -I .

#include "tests/cxx/mapping_to_self.h"

/**** IWYU_SUMMARY

(tests/cxx/mapping_to_self.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

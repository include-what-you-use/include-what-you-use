//===--- internal_files.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests our handling of header files in /internal/.  In particular,
// if the only includers are outside the internal directory, make sure
// we don't map the internal include to <built-in>.

#include "tests/cxx/internal/private.h"

InternalStruct is;


/**** IWYU_SUMMARY

(tests/cxx/internal/internal_files.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

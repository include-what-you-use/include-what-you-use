//===--- external_including_internal.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when a non-internal file #includes an internal file,
// that we don't try to map the include back to ourself, and protects
// against a regression of a bug where we were both including a file
// in /internal/, and forward-declaring a symbol it provided.

#include "tests/internal/private.h"

InternalStruct is;
InternalStruct* isp;


/**** IWYU_SUMMARY

(tests/external_including_internal.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

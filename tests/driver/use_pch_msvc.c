//===--- use_pch_msvc.c - test input file for IWYU ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU fails for MSVC spelling of PCH args.

// IWYU_ARGS: --driver-mode=cl /I . /Yutests/driver/use_pch_msvc.h

// IWYU~: include-what-you-use does not support PCH

#include "tests/driver/use_pch_msvc.h"

/**** IWYU_SUMMARY(1)

// No IWYU summary expected.

***** IWYU_SUMMARY */

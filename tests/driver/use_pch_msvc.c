//===--- use_pch_msvc.c - test input file for IWYU ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU fails for MSVC spelling of PCH args.

// IWYU_ARGS: --driver-mode=cl /I . /Yu tests/driver/use_pch_msvc.h

// Clang for some reason creates a precompiler job when /Yu (use PCH) is
// present. We filter it out, but warn, so expect that warning.
// IWYU~: ignoring unsupported job type: precompiler

// IWYU~: include-what-you-use does not support PCH

/**** IWYU_SUMMARY(1)

// No IWYU summary expected.

***** IWYU_SUMMARY */

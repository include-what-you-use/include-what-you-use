//===--- use_pch.c - test input file for IWYU -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU fails when user attempts to bring in a PCH using
// -include-pch.

// IWYU_ARGS: -include-pch some.pch

// IWYU~: include-what-you-use does not support PCH

/**** IWYU_SUMMARY(1)

// No IWYU summary expected.

***** IWYU_SUMMARY */

//===--- generate_pch.c - test input file for IWYU ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU ignores the 'precompile' job produced by a PCH-generating
// command. Since that's the only job, it proceeds to fail because there's no
// compiler job to replace.

// IWYU_ARGS: -o generate_pch.c.pch -xc-header

// IWYU~: ignoring unsupported job type.*precompile
// IWYU~: expected exactly one compiler job

/**** IWYU_SUMMARY(1)

// No IWYU summary expected.

***** IWYU_SUMMARY */

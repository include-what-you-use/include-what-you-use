//===--- generate_pch.c - test input file for IWYU ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU uses the 'precompile' job produced by a PCH-generating
// command.

// IWYU_ARGS: -o generate_pch.c.pch -xc-header

/**** IWYU_SUMMARY(0)

(tests/driver/generate_pch.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

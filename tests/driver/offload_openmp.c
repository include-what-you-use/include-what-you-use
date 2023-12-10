//===--- offload_openmp.c - test input file for IWYU ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU ignores the extra offload compiler job produced when
// compiling for OpenMP.

// IWYU_ARGS: -fopenmp -fopenmp-targets=nvptx64 -nocudalib

// This first diagnostic only happens because I don't have an nvptx64 toolchain
// on my machine -- this test should maybe be conditional on the presence of
// such a toolchain.
// IWYU~: Executable ".*" doesn't exist!; consider passing it via '-march'

// IWYU~: ignoring offload job for device toolchain: openmp

/**** IWYU_SUMMARY(0)

(tests/driver/offload_openmp.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

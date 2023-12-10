//===--- offload_apple.c - test input file for IWYU -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that IWYU ignores the extra offload compiler job produced when
// compiling for Apple's HIP.

// IWYU_ARGS: -target arm64-apple-macosx11.0.0 -x hip -nogpulib -nogpuinc

// IWYU~: ignoring offload job for device toolchain: hip

/**** IWYU_SUMMARY(0)

(tests/driver/offload_apple.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

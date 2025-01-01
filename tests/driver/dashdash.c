//===--- dashdash.c - test input file for ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Clang uses '--' to separate flags from input args. Make sure IWYU doesn't put
// implicit arguments after '--'.

// IWYU_ARGS: -c --

int main() {
  return 0;
}

/**** IWYU_SUMMARY

(tests/driver/dashdash.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

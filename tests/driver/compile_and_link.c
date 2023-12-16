//===--- compile_and_link.c - test input file for -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The presence of -o and -l would nominally create a linker job. That is
// filtered out implicitly by -fsyntax-only, which means nobody claims the
// -lblah switch. Check that IWYU doesn't warn about ''linker' input unused'.

// IWYU_ARGS: -o program -lblah

int main() {
  return 0;
}

/**** IWYU_SUMMARY

(tests/driver/compile_and_link.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

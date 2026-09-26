//===--- vector_attr.c - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Very basic test to show that IWYU doesn't choke on constructs decorated with
// vector attributes.

typedef float v4f __attribute__((ext_vector_type(4)));

v4f vector_splat(float value) {
  // Triggers a special vector-splat cast.
  return (v4f)value;
}

/**** IWYU_SUMMARY

(tests/c/vector_attr.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

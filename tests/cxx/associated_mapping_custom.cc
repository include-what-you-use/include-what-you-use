//===--- associated_mapping_custom.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/cxx/associated_mapping_custom.inline -Xiwyu --check_also=tests/cxx/associated_mapping_custom.inline -Xiwyu --mapping_file=tests/cxx/associated_mapping_custom.imp -I .

// The 'associated' mapping directive is about pairs of paths, not about a
// fixed set of extensions: a project whose inline definitions live in
// foo.inline (or foo_impl.h, or detail/foo.hpp) gets the same treatment as
// one using foo.icc, by naming its own convention in a mapping file.

#include "tests/cxx/associated_mapping_custom.h"

void Use() {
  CustomInlineClass c;
  c.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_mapping_custom.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

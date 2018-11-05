//===--- fwd_decl_without_full_use.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests a situation where iwyu tries to remove an important fwd-decl
// when --no_fwd_decls is enabled.

class Foo;

void foo_fn(const Foo* foo) {}


/**** IWYU_SUMMARY

(tests/cxx/fwd_decl_without_full_use.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

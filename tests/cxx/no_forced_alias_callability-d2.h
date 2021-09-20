//===--- no_forced_alias_callability-d2.h - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct Aliased;

typedef Aliased Alias;

/**** IWYU_SUMMARY

(tests/cxx/no_forced_alias_callability-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

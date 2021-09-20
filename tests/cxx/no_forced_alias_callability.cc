//===--- no_forced_alias_callability.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/no_forced_alias_callability-d2.h" \
//            -I .

// Tests that IWYU doesn't require inclusion of an aliased class header
// (...-d1.h) into a header with the alias to provide callability of methods
// of the aliased class if the aliased class is explicitly made forward declared
// in accordance with the IWYU policy

#include "tests/cxx/no_forced_alias_callability-d1.h"
#include "tests/cxx/no_forced_alias_callability-d2.h"

int main() {
  Alias a;
}

/**** IWYU_SUMMARY

(tests/cxx/no_forced_alias_callability.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

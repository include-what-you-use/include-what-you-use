//===--- associated_pragma_order.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//            -Xiwyu --check_also=tests/cxx/associated_pragma_order-d1.h

// Tests that a header's associated header is analyzed before the header,
// whatever the order in which IWYU first sees them: -d1.h marks -i1.h as
// its associated header with a pragma.

#include "tests/cxx/associated_pragma_order-d1.h"

int Use() {
  return UseD1();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_pragma_order.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

//===--- header_near.cc - test input file for iwyu ------------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Not including as "tests/header_near.h" is done on purpose. We are actually
// testing that iwyu doesn't offer to #include "tests/header_near.h" in addition
// to existing #include "header_near.h"
//
// The same for "indirect.h" - need to test both associated and usual includes.

#include "header_near.h"
#include "indirect.h"

Foo::Foo() {
 IndirectClass ic;
}

/**** IWYU_SUMMARY

(tests/header_near.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

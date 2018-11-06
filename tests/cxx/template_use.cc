//===--- template_use.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test is here because IWYU may incorrectly suggest to delete
// this include.

#include "template_use.h"

class Bar{
    Foo<int> foo;
};




/**** IWYU_SUMMARY
(tests/cxx/template_use.cc has correct #includes/fwd-decls)
***** IWYU_SUMMARY */

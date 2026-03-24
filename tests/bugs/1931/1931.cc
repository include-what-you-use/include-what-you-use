//===--- 1931.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "1931.h"

#include <iostream>

void A::print()
{
    std::cout << "A\n";
}

/**** IWYU_SUMMARY

(tests/bugs/1931/1931.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

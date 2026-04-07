//===--- 1789.c - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "direct.h"

int main() {
    return var_direct.i;
}

/**** IWYU_SUMMARY

(tests/bugs/1789/1789.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

//===--- 135.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "myvec.h"

int main() {
    myvec v; // is fine
    myvec::const_iterator p; // causes iwyu to claim vector is needed
}

/**** IWYU_SUMMARY

(tests/bugs/135/135.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

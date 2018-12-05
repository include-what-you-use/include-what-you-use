//===--- symbol_uses.c - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests if macro used in a header has it's defining header included
#include "symbol_uses_b.h"
#include "symbol_uses_a.h"

void t() {
	int a = A;
}

/**** IWYU_SUMMARY

(tests/c/symbol_uses.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

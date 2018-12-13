//===--- include_order.c - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "include_order_c.h"
void foo() {
  int t = a + B;
}

/**** IWYU_SUMMARY

include_order.c should add these lines:
#include "include_order_b.h"  // for B
#include "include_order_a.h"  // for a

include_order.c should remove these lines:
- #include "include_order_c.h"  // lines 1-1

The full include-list for include_order.c:
#include "include_order_b.h"  // for B
#include "include_order_a.h"  // for a

***** IWYU_SUMMARY */


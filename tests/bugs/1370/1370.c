//===--- 1370.c - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "b.h"

struct SymbolC {
  struct SymbolA* a;
  struct SymbolB* b;
};

int main(void) {
  // IWYU: SymbolA is...*a.h
  struct SymbolA a = {.eh = 0};
  struct SymbolB b = {.a = a};
  const struct SymbolC c = {.a = &a, .b = &b};
  (void)c;

  return 0;
}

/**** IWYU_SUMMARY

tests/bugs/1370/1370.c should add these lines:
#include "a.h"

tests/bugs/1370/1370.c should remove these lines:

The full include-list for tests/bugs/1370/1370.c:
#include "a.h"  // for SymbolA
#include "b.h"  // for SymbolB

***** IWYU_SUMMARY */

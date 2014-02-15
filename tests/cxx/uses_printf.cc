//===--- uses_printf.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/uses_printf-d1.h"

void hello() {
  // IWYU: printf is...*<stdio.h>
  printf("Hello, world!\n");
}

/**** IWYU_SUMMARY

tests/cxx/uses_printf.cc should add these lines:
#include <stdio.h>

tests/cxx/uses_printf.cc should remove these lines:
- #include "tests/cxx/uses_printf-d1.h"  // lines XX-XX

The full include-list for tests/cxx/uses_printf.cc:
#include <stdio.h>  // for printf

***** IWYU_SUMMARY */

//===--- elaborated_struct.c - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that elaborated types suppressing the need for forward declarations is
// a C++-only feature. In C, we need a forward declaration, elaboration or not.
#include "tests/c/elaborated_struct-d1.h"

int UseStruct(struct Struct* s);

/**** IWYU_SUMMARY

tests/c/elaborated_struct.c should add these lines:
struct Struct;

tests/c/elaborated_struct.c should remove these lines:
- #include "tests/c/elaborated_struct-d1.h"  // lines XX-XX

The full include-list for tests/c/elaborated_struct.c:
struct Struct;

***** IWYU_SUMMARY */

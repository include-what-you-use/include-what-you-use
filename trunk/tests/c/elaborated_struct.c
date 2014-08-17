//===--- elaborated_struct.c - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/c/elaborated_struct-d1.h"

// C basically never requires an explicit forward declaration, all uses of
// structs are elaborated.
struct AnotherStruct* s = 0;  // No diagnostic expected.
struct YetAnotherStruct* ReturnAStruct();  // No diagnostic expected.
typedef struct TypedeffedStruct TypedeffedStruct;  // No diagnostic expected.

// However, if an elaborated struct declaration appears in the parameter list
// of a function declaration, Clang will throw a warning that the declaration
// won't be visible outside the function. We avoid that warning by suggesting
// that an explicit forward declaration would be better.
int UseStruct(struct Struct* s);

/**** IWYU_SUMMARY

tests/c/elaborated_struct.c should add these lines:
struct Struct;

tests/c/elaborated_struct.c should remove these lines:
- #include "tests/c/elaborated_struct-d1.h"  // lines XX-XX

The full include-list for tests/c/elaborated_struct.c:
struct Struct;

***** IWYU_SUMMARY */

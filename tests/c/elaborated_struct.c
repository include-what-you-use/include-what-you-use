//===--- elaborated_struct.c - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

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

// If an existing forward-declaration is available, make sure we don't suggest
// adding it twice (see issue #682).
struct ForwardDeclared;
void UseForwardDeclared(struct ForwardDeclared*);

// If a forward-declaration is seen before an actual struct declaration in the
// same file, no diagnostic is expected (see issue #1065).
typedef struct local_struct local_struct_t;
struct local_struct {
  int x;
  local_struct_t* next;
};

/**** IWYU_SUMMARY

tests/c/elaborated_struct.c should add these lines:
struct Struct;

tests/c/elaborated_struct.c should remove these lines:
- #include "tests/c/elaborated_struct-d1.h"  // lines XX-XX

The full include-list for tests/c/elaborated_struct.c:
struct ForwardDeclared;  // lines XX-XX
struct Struct;

***** IWYU_SUMMARY */

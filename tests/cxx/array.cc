//===--- array.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests handling C-style arrays. In particular, tests that we handle correctly
// a[i] as a full use of a.

#include "tests/cxx/direct.h"

class A {
  // IWYU: IndirectClass needs a declaration
  IndirectClass *getIndirectClass(int i) {
    // IWYU: IndirectClass is...*indirect.h
    (void)sizeof(b[i]);  // requires full type
    // IWYU: IndirectClass is...*indirect.h
    (void)sizeof(&(b[i]));  // requires full type

    // Neither fwd-declaration nor full type is needed for array of pointers
    // indexing and pointer size taking.
    (void)sizeof(pp[i]);

    // IWYU: IndirectClass is...*indirect.h
    return &(b[i]);
  }

  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  IndirectTemplate<IndirectClass> *getIndirectTemplateSpecialization(int i) {
    // IWYU: IndirectTemplate is...*indirect.h
    // IWYU: IndirectClass is...*indirect.h
    (void)sizeof(t[i]);  // requires full type
    // IWYU: IndirectTemplate is...*indirect.h
    // IWYU: IndirectClass is...*indirect.h
    (void)sizeof(&(t[i]));  // requires full type

    // Neither fwd-declaration nor full type is needed for array of pointers
    // indexing and pointer size taking.
    (void)sizeof(ppt[i]);

    // IWYU: IndirectTemplate is...*indirect.h
    // IWYU: IndirectClass is...*indirect.h
    return &(t[i]);
  }

  // IWYU: IndirectClass needs a declaration
  IndirectClass *b;
  // IWYU: IndirectClass needs a declaration
  IndirectClass **pp;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  IndirectTemplate<IndirectClass> *t;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  IndirectTemplate<IndirectClass> **ppt;
};

// No need of full type info for external variable declarations.
// IWYU: IndirectClass needs a declaration
extern IndirectClass extern_array[5];
// IWYU: IndirectClass needs a declaration
extern IndirectClass extern_multidim_array[5][6][7];

// IWYU: IndirectClass is...*indirect.h
IndirectClass defined_array1[5];
// IWYU: IndirectClass is...*indirect.h
decltype(extern_array) defined_array2;
// IWYU: IndirectClass is...*indirect.h
IndirectClass defined_multidim_array1[5][6][7];
// IWYU: IndirectClass is...*indirect.h
decltype(extern_multidim_array) defined_multidim_array2;

// No need of full type for the parameter: it's just a pointer in fact.
// IWYU: IndirectClass needs a declaration
void IncompleteArrayTypeParamFn(IndirectClass param[]) {
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(IndirectClass[5]);
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(extern_array);
  // IWYU: IndirectClass is...*indirect.h
  (void)extern_array[1];

  // IWYU: IndirectClass needs a declaration
  IndirectClass* pointer_array[5];
  (void)pointer_array[1];
  (void)sizeof(pointer_array);

  // IWYU: IndirectClass needs a declaration
  IndirectClass(*array_pointer)[5];
  // IWYU: IndirectClass is...*indirect.h
  (void)array_pointer[0];

  // IWYU: IndirectClass needs a declaration
  IndirectClass(((*test_reaching_pointer_through_parens_and_arrays)[7][6])[5]);
}

// This is a confusing and harmful way of writing parameter of a pointer type,
// but compilers accept it.
// IWYU: IndirectClass needs a declaration
void ConstantArrayTypeParamFn(IndirectClass param[5]) {
}

// No need to report anything but extern_array, which is already in this file.
void DecltypeParamFn(decltype(extern_array)) {
}

/**** IWYU_SUMMARY

tests/cxx/array.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/array.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/array.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */

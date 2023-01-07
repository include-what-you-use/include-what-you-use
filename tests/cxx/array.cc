//===--- array.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we handle correctly identify a[i] as a full use of a.

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


/**** IWYU_SUMMARY

tests/cxx/array.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/array.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/array.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */

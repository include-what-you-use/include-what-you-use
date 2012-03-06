//===--- sizeof_reference.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that sizeof(reference) is treated the same as
// sizeof(underlying_object), like it's supposed to be.

#include <stddef.h>
#include "tests/direct.h"

template <typename T> struct IndirectTemplateStruct {
  T value;   // require full type information for t;
};

template <typename T> struct SizeofTakingStruct {
  char value[sizeof(T)];
};

template <typename T> struct SizeofTakingStructRef {
  char value[sizeof(T&)];
};

template <typename T> struct SizeofTakingStructTpl {
  char value[sizeof(IndirectTemplateStruct<T>)];
};

template <typename T> struct SizeofTakingStructTplRef {
  char value[sizeof(IndirectTemplateStruct<T&>)];
};

template <typename T> struct SizeofTakingStructTplRef2 {
  char value5[sizeof(IndirectTemplateStruct<T>&)];
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
size_t s = sizeof(IndirectClass&);

// This needs the full type of IndirectTemplateStruct, but also
// IndirectClass, which is used in the IndirectTemplateStruct
// instantiation.
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
size_t s2 = sizeof(IndirectTemplateStruct<IndirectClass>&);

// This does not need the full type information for IndirectClass.
// IWYU: IndirectClass needs a declaration
size_t s3 = sizeof(IndirectTemplateStruct<IndirectClass&>);

// This needs the full type information of IndirectClass because the
// subclass takes the sizeof().
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
size_t s4 = sizeof(SizeofTakingStruct<IndirectClass&>);

// Also check when sizeof is on a variable, not a type.
// IWYU: IndirectClass is...*indirect.h
IndirectClass dummy;
// IWYU: IndirectClass needs a declaration
IndirectClass& ref = dummy;
// IWYU: IndirectClass is...*indirect.h
size_t s5 = sizeof(dummy);

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SizeofTakingStruct<IndirectClass&> sizeof_taking_struct1;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SizeofTakingStructRef<IndirectClass> sizeof_taking_struct2;

// Not sure why, but C++ doesn't require full type of IndirectClass here.
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTpl<IndirectClass&> sizeof_taking_struct3;

// Not sure why, but C++ doesn't require full type of IndirectClass here.
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef<IndirectClass> sizeof_taking_struct4;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef2<IndirectClass> sizeof_taking_struct5;

// Not sure why, but C++ doesn't require full type of IndirectClass here.
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef2<IndirectClass&> sizeof_taking_struct6;


/**** IWYU_SUMMARY

tests/sizeof_reference.cc should add these lines:
#include "tests/indirect.h"

tests/sizeof_reference.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/sizeof_reference.cc:
#include <stddef.h>  // for size_t
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */

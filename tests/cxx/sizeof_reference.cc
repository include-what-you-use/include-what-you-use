//===--- sizeof_reference.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that sizeof(reference) is treated the same as
// sizeof(underlying_object), like it's supposed to be.
//
// C++ [expr.sizeof]p2:
//   When applied to a reference or a reference type,
//   the result is the size of the referenced type.

#include <stddef.h>
#include "tests/cxx/direct.h"

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

// sizeof(IndirectTemplateStruct<IndirectClass&>) doesn't require IndirectClass
// full type because IndirectTemplateStruct<IndirectClass&> stores just
// a pointer, in fact. Hence, its size doesn't depend on IndirectClass size.
// C++ [expr.sizeof]p2:
//   When applied to a class, the result is the number of bytes in an object of
//   that class...
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTpl<IndirectClass&> sizeof_taking_struct3;

// sizeof(IndirectTemplateStruct<IndirectClass&>) doesn't require IndirectClass
// full type.
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef<IndirectClass> sizeof_taking_struct4;

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef2<IndirectClass> sizeof_taking_struct5;

// sizeof(IndirectTemplateStruct<IndirectClass&>) doesn't require IndirectClass
// full type.
// IWYU: IndirectClass needs a declaration
SizeofTakingStructTplRef2<IndirectClass&> sizeof_taking_struct6;

// The same with some sugar.

// IWYU: IndirectClass is...*indirect.h
SizeofTakingStruct<decltype(ref)> sizeof_taking_struct7;

// IWYU: IndirectClass is...*indirect.h
SizeofTakingStructRef<decltype(dummy)> sizeof_taking_struct8;

SizeofTakingStructTpl<decltype(ref)> sizeof_taking_struct9;

SizeofTakingStructTplRef<decltype(dummy)> sizeof_taking_struct10;

// IWYU: IndirectClass is...*indirect.h
SizeofTakingStructTplRef2<decltype(dummy)> sizeof_taking_struct11;

SizeofTakingStructTplRef2<decltype(ref)> sizeof_taking_struct12;

/**** IWYU_SUMMARY

tests/cxx/sizeof_reference.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/sizeof_reference.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/sizeof_reference.cc:
#include <stddef.h>  // for size_t
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */

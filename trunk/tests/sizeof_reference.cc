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
#include "tests/sizeof_reference-d1.h"

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
size_t s = sizeof(IndirectClass&);

// This needs the full type of IndirectTemplateStruct, but also
// IndirectClass, which is used in the IndirectTemplateStruct
// instantiation.
// IWYU: IndirectTemplateStruct is...*sizeof_reference-i1.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectTemplateStruct needs a declaration
// IWYU: IndirectClass needs a declaration
size_t s2 = sizeof(IndirectTemplateStruct<IndirectClass>&);

// This does not need the full type information for IndirectClass.
// IWYU: IndirectTemplateStruct is...*sizeof_reference-i1.h
// IWYU: IndirectClass needs a declaration
size_t s3 = sizeof(IndirectTemplateStruct<IndirectClass&>);

// This needs the full type information of IndirectClass because the
// subclass takes the sizeof().
// IWYU: SizeofTakingStruct is...*sizeof_reference-i1.h
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

// IWYU: SizeofTakingStruct is...*sizeof_reference-i1.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SizeofTakingStruct<IndirectClass&> sizeof_taking_struct;

/**** IWYU_SUMMARY

tests/sizeof_reference.cc should add these lines:
#include "tests/indirect.h"
#include "tests/sizeof_reference-i1.h"

tests/sizeof_reference.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX
- #include "tests/sizeof_reference-d1.h"  // lines XX-XX

The full include-list for tests/sizeof_reference.cc:
#include <stddef.h>  // for size_t
#include "tests/indirect.h"  // for IndirectClass
#include "tests/sizeof_reference-i1.h"  // for IndirectTemplateStruct, SizeofTakingStruct

***** IWYU_SUMMARY */

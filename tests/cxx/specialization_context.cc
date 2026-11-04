//===--- specialization_context.cc - test input ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/direct.h"
#include "tests/cxx/specialization_context-direct.h"

// IWYU: PointerThenValue is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
PointerThenValue<IndirectClass> pointer_then_value;

// IWYU: ValueThenPointer is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
ValueThenPointer<IndirectClass> value_then_pointer;

// IWYU: DirectPointerThenValue is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
DirectPointerThenValue<IndirectClass> direct_pointer_then_value;

// IWYU: DirectValueThenPointer is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
DirectValueThenPointer<IndirectClass> direct_value_then_pointer;

// Shared alias arguments avoid relying on separately written Box<T> nodes.
// IWYU: SharedPointerFirst is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SharedPointerFirst<IndirectClass> shared_pointer_first;

// IWYU: SharedValueFirst is...*specialization_context-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
SharedValueFirst<IndirectClass> shared_value_first;

/**** IWYU_SUMMARY

tests/cxx/specialization_context.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/specialization_context-indirect.h"

tests/cxx/specialization_context.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_context-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_context.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/specialization_context-indirect.h"  // for DirectPointerThenValue, DirectValueThenPointer, PointerThenValue, SharedPointerFirst, SharedValueFirst, ValueThenPointer

***** IWYU_SUMMARY */

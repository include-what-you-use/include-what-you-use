//===--- placement_new.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++17 -I .

// Test that use of placement-new requires include of <new> in all the usual
// scenarios.
//
// Requires -std=c++17 on the command-line to have std::aligned_val_t available
// in <new>.

#include "tests/cxx/direct.h"
#include "tests/cxx/placement_new-d1.h"

// Placement new of builtin types.
void PlacementNewBuiltinType() {
  // Make sure we only report a use of <new> because of placement new, not
  // ordinary new-expressions.
  int* newed_int = new int;
  // IWYU: operator new is...*<new>
  new (newed_int) int(4);

  delete newed_int;
}

// Placement new of user-defined type.
void PlacementNewUserType() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass* icptr = new IndirectClass;

  // IWYU: IndirectClass is...*indirect.h
  // IWYU: operator new is...*<new>
  new (icptr) IndirectClass;

  // IWYU: IndirectClass is...*indirect.h
  delete icptr;
}

// Placement new in macro, use is attributed to the macro.
static char global_buffer[256];
// IWYU: operator new is...*<new>
#define CONSTRUCT_GLOBAL(T) new (global_buffer) T;

void PlacementNewInMacro() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass* a = CONSTRUCT_GLOBAL(IndirectClass);
}

// Placement new inside a template.
template <typename T>
void PlacementNewInTemplate(T t) {
  static char buffer[sizeof(t)];
  // These should all be iwyu violations here, even though we can't be
  // *sure* some of these are actually placement new until we get a
  // specific type for T (at template-instantiation time).
  // IWYU: operator new is...*<new>
  new (&t) int;
  // IWYU: operator new is...*<new>
  new (buffer) T();
  // IWYU: operator new is...*<new>
  new (&t) T();
}

// Placement new when the newed type _is_ a template.
void PlacementNewOfTemplate() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: ClassTemplate is...*placement_new-i1.h
  char template_storage[sizeof(ClassTemplate<IndirectClass, IndirectClass>)];

  // IWYU: IndirectClass needs a declaration
  // IWYU: ClassTemplate needs a declaration
  ClassTemplate<IndirectClass, IndirectClass>* placement_newed_template =
      // Need <new> because of placement new, and requires both template and
      // arguments as complete types.
      //
      // IWYU: IndirectClass needs a declaration
      // IWYU: IndirectClass is...*indirect.h
      // IWYU: ClassTemplate is...*placement_new-i1.h
      // IWYU: operator new is...*<new>
      new (template_storage) ClassTemplate<IndirectClass, IndirectClass>();

  // Make sure we handle it right when we explicitly call the dtor, as well.
  // IWYU: ClassTemplate is...*placement_new-i1.h
  // IWYU: IndirectClass is...*indirect.h
  placement_newed_template->~ClassTemplate();
}

// new(std::nothrow) is not strictly placement allocation, but it uses placement
// syntax to adjust exception policy.
// To use 'std::nothrow' we must include <new>, even if we don't need it for
// 'new' itself.
void NoThrow() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: std::nothrow is...*<new>
  IndirectClass* elem = new (std::nothrow) IndirectClass;
  // IWYU: IndirectClass is...*indirect.h
  delete elem;

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: std::nothrow is...*<new>
  IndirectClass* arr = new (std::nothrow) IndirectClass[4];
  // IWYU: IndirectClass is...*indirect.h
  delete[] arr;
}

// new(std::align_val_t) is not strictly placement allocation, but it uses
// placement syntax to provide alignment hints.
// To use 'std::align_val_t' we must include <new>, even if we don't need it
// for 'new' itself.
// The aligned allocation mechanics are only available as of C++17.
void ExplicitAlignedAllocation() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: std::align_val_t is...*<new>
  IndirectClass* elem = new (std::align_val_t(32)) IndirectClass;
  // IWYU: IndirectClass is...*indirect.h
  delete elem;

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: std::align_val_t is...*<new>
  IndirectClass* arr = new (std::align_val_t(32)) IndirectClass[10];
  // IWYU: IndirectClass is...*indirect.h
  delete[] arr;
}

/**** IWYU_SUMMARY

tests/cxx/placement_new.cc should add these lines:
#include <new>
#include "tests/cxx/indirect.h"
#include "tests/cxx/placement_new-i1.h"

tests/cxx/placement_new.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/placement_new-d1.h"  // lines XX-XX

The full include-list for tests/cxx/placement_new.cc:
#include <new>  // for align_val_t, nothrow, operator new
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/placement_new-i1.h"  // for ClassTemplate

***** IWYU_SUMMARY */

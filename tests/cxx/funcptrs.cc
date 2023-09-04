//===--- funcptrs.cc - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-unused -I .

// Tests that function pointers make the right claims for involved types.
// Function pointer expressions come in three flavors:
//
// 1) Assignments: int (*fptr)(int) = function;
// 2) Calls: FunctionThatTakesFptr(function);
// 3) Naked expressions: &function;
//
// A 'function' can be a free function, a static member function, a member
// function, or any template instantiation of the above.

#include "tests/cxx/direct.h"
#include "tests/cxx/funcptrs-d1.h"


// Functions to drive call-syntax.

// IWYU: Class needs a declaration
// IWYU: Enum is...*funcptrs-i1.h
void FunctionThatTakesFptr(Enum (*fptr)(Class*));

// IWYU: Class needs a declaration
void FunctionThatTakesFptr(int (*fptr)(Class*));

void FunctionThatTakesFptr(int (*fptr)());

// IWYU: Class needs a declaration
void FunctionThatTakesMptr(int (Class::*mptr)() const);

// IWYU: ClassTemplate needs a declaration
// IWYU: Class needs a declaration
void FunctionThatTakesMptr(int (ClassTemplate<Class>::*mptr)() const);


// Test cases below.
// Note that we test primarily the diagnostics from IWYU for the individual
// constructs, not which header is chosen -- all relevant types are in
// funcptrs-i1.h anyway.
//
// Each test creates function pointers to a plain function and a template
// instantiation, and for classes similarly for instance member functions.

void FreeFunctions() {
  // Assignment of function pointer to function and template instantiation.
  // IWYU: Class needs a declaration
  // IWYU: Enum is...*funcptrs-i1.h
  // IWYU: Function is...*funcptrs-i1.h
  Enum (*fptr)(Class*) = &Function;

  // IWYU: Class needs a declaration
  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: FunctionTemplate is...*funcptrs-i1.h
  int (*template_fptr)(Class*) = &FunctionTemplate<Retval>;

  // Call with function pointer to function and template instantiation.
  // IWYU: Function is...*funcptrs-i1.h
  FunctionThatTakesFptr(Function);

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: FunctionTemplate is...*funcptrs-i1.h
  FunctionThatTakesFptr(FunctionTemplate<Retval>);

  // Naked function pointer expressions
  // IWYU: Function is...*funcptrs-i1.h
  &Function;

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: FunctionTemplate is...*funcptrs-i1.h
  &FunctionTemplate<Retval>;

  // Full return type info is needed neither for non-templated function...
  // IWYU: FunctionReturningRecordType is...*funcptrs-i1.h
  &FunctionReturningRecordType;
  // ... nor for function template implicit instantiation without definition...
  // IWYU: FunctionTemplate2 is...*funcptrs-i1.h
  &FunctionTemplate2<char>;
  // ... nor for function template explicit specialization or instantiation
  // references.
  // IWYU: FunctionTemplate2 is...*funcptrs-i1.h
  &FunctionTemplate2<int>;
}

void ClassMembers() {
  // IWYU: Class is...*funcptrs-i1.h
  int (*static_method_ptr)() = &Class::StaticMemberFunction;

  // IWYU: Class is...*funcptrs-i1.h
  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  int (*static_template_method_ptr)() = &Class::StaticMemberTemplate<Retval>;

  // IWYU: Class is...*funcptrs-i1.h
  int (Class::*method_ptr)() const = &Class::MemberFunction;

  // IWYU: Class is...*funcptrs-i1.h
  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  int (Class::*template_method_ptr)() const = &Class::MemberTemplate<Retval>;

  // Call with pointers to static member function and template instantiation.
  // IWYU: Class is...*funcptrs-i1.h
  FunctionThatTakesFptr(Class::StaticMemberFunction);

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: Class is...*funcptrs-i1.h
  FunctionThatTakesFptr(Class::StaticMemberTemplate<Retval>);

  // Call with pointers to instance member function and template instantiation.
  // IWYU: Class is...*funcptrs-i1.h
  FunctionThatTakesMptr(&Class::MemberFunction);

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: Class is...*funcptrs-i1.h
  FunctionThatTakesMptr(&Class::MemberTemplate<Retval>);

  // Naked function pointer expressions
  // IWYU: Class is...*funcptrs-i1.h
  &Class::StaticMemberFunction;

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: Class is...*funcptrs-i1.h
  &Class::StaticMemberTemplate<Retval>;

  // IWYU: Class is...*funcptrs-i1.h
  &Class::MemberFunction;

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: Class is...*funcptrs-i1.h
  &Class::MemberTemplate<Retval>;
}

void ClassTemplateMembers() {
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  int (*static_method_ptr)() = &ClassTemplate<Class>::StaticMemberFunction;

  int (*static_template_method_ptr)() =
      // IWYU: ClassTemplate is...*funcptrs-i1.h
      // IWYU: Class needs a declaration
      // IWYU: Retval needs a declaration
      // IWYU: Retval is...*funcptrs-i1.h
      &ClassTemplate<Class>::StaticMemberTemplate<Retval>;

  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  int (ClassTemplate<Class>::*method_ptr)() const =
      // IWYU: ClassTemplate is...*funcptrs-i1.h
      // IWYU: Class needs a declaration
      &ClassTemplate<Class>::MemberFunction;

  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  int (ClassTemplate<Class>::*template_method_ptr)() const =
      // IWYU: ClassTemplate is...*funcptrs-i1.h
      // IWYU: Class needs a declaration
      // IWYU: Retval needs a declaration
      // IWYU: Retval is...*funcptrs-i1.h
      &ClassTemplate<Class>::MemberTemplate<Retval>;

  // Call with pointers to static member function and template instantiation.
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  FunctionThatTakesFptr(ClassTemplate<Class>::StaticMemberFunction);

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  FunctionThatTakesFptr(ClassTemplate<Class>::StaticMemberTemplate<Retval>);

  // Call with pointers to instance member function and template instantiation.
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  FunctionThatTakesMptr(&ClassTemplate<Class>::MemberFunction);

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  FunctionThatTakesMptr(&ClassTemplate<Class>::MemberTemplate<Retval>);

  // Naked class template member function pointer expressions
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  &ClassTemplate<Class>::StaticMemberFunction;

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  &ClassTemplate<Class>::StaticMemberTemplate<Retval>;

  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  &ClassTemplate<Class>::MemberFunction;

  // IWYU: Retval needs a declaration
  // IWYU: Retval is...*funcptrs-i1.h
  // IWYU: ClassTemplate is...*funcptrs-i1.h
  // IWYU: Class needs a declaration
  &ClassTemplate<Class>::MemberTemplate<Retval>;
}

/**** IWYU_SUMMARY

tests/cxx/funcptrs.cc should add these lines:
#include "tests/cxx/funcptrs-i1.h"

tests/cxx/funcptrs.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/funcptrs-d1.h"  // lines XX-XX

The full include-list for tests/cxx/funcptrs.cc:
#include "tests/cxx/funcptrs-i1.h"  // for Class, ClassTemplate, Enum, Function, FunctionReturningRecordType, FunctionTemplate, FunctionTemplate2, Retval

***** IWYU_SUMMARY */

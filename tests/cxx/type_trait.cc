//===--- type_trait.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/type_trait-d1.h"
#include "tests/cxx/type_trait-d2.h"

// To avoid lots of "needs a declaration" warnings.
class Class;
class StructDerivedClass;
struct Struct;
union Union1;
union Union2;

// clang doesn't require full type info for types used in "convertible" traits.
// The C++ standard requires complete types or unbounded arrays. Despite
// pointers and references are always complete, it is better to suggest the full
// pointed-to type in certain cases to produce stable result and thus to avoid
// a UB.

// IWYU: Base needs a declaration
// IWYU: Derived is...*type_trait-i2.h
// IWYU: Derived needs a declaration
static_assert(__is_convertible_to(Derived*, Base*),
              "Derived should be convertible to the Base class");

// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_convertible_to(Derived**, Base**),
              "Indirect pointers shouldn't be convertible");

// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
// IWYU: Derived needs a declaration
static_assert(__is_convertible_to(Derived&, Base&),
              "Derived should be convertible to the Base class");

// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Class, int));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Class, int));
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible(int, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_convertible(int, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_convertible(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible(int, Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_convertible(int, Class[5]));
// TODO: no need of full type for arrays of unknown bound.
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible(int, Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_convertible(int, Class[]));
// When the pointed-to types are the same, user-defined conversions are not
// considered, hence the complete type is not needed.
static_assert(__is_convertible(Class&, Class&));
static_assert(__is_nothrow_convertible(Class&, Class&));
static_assert(!__is_convertible(Class&, Class&&));
static_assert(!__is_nothrow_convertible(Class&, Class&&));
static_assert(!__is_convertible(Class&&, Class&));
static_assert(!__is_nothrow_convertible(Class&&, Class&));
static_assert(__is_convertible(Class&&, const Class&));
static_assert(__is_nothrow_convertible(Class&&, const Class&));
static_assert(__is_convertible(Class*, Class*));
static_assert(__is_nothrow_convertible(Class*, Class*));
static_assert(!__is_convertible(const Class*, Class*));
static_assert(!__is_nothrow_convertible(const Class*, Class*));
static_assert(__is_convertible(Class* const, Class*));
static_assert(__is_nothrow_convertible(Class* const, Class*));
static_assert(__is_convertible(Class*&, Class*));
static_assert(__is_nothrow_convertible(Class*&, Class*));
static_assert(__is_convertible(Class * &&, const Class* const&));
static_assert(__is_nothrow_convertible(Class * &&, const Class* const&));
static_assert(__is_convertible(ClassRefNonProviding, Class&));
static_assert(__is_nothrow_convertible(ClassRefNonProviding, Class&));
static_assert(__is_convertible(Class&, ClassRefNonProviding));
static_assert(__is_nothrow_convertible(Class&, ClassRefNonProviding));
static_assert(__is_convertible(ClassNonProviding&, Class&));
static_assert(__is_nothrow_convertible(ClassNonProviding&, Class&));
static_assert(__is_convertible(Class&, ClassNonProviding&));
static_assert(__is_nothrow_convertible(Class&, ClassNonProviding&));
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Class&, int));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Class&, int));
// Struct has the conversion operator to Class& (or could be a class derived
// from Class), but the details of Class type are irrelevant for non-const
// lvalue reference case, because its constructors cannot be involved.
// TODO: Struct may have a conversion operator to the reference to a class
// derived from Class. It should be reported.
// IWYU: Struct is...*-i1.h
static_assert(__is_convertible(Struct&, Class&));
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_convertible(Struct&, Class&));
// 'const Class&' may bind to a Class temporary object, so its constructors are
// to be considered, in general.
// IWYU: Struct is...*-i1.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Struct&, const Class&));
// IWYU: Struct is...*-i1.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Struct&, const Class&));
// Class has constructors from Base& and Base*, so the complete Derived type is
// required to detect the inheritance.
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Derived&, const Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Derived&, const Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Derived*, const Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Derived*, const Class&));
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(DerivedRefNonProviding,
                               ClassConstRefNonProviding));
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(DerivedRefNonProviding,
                                       ClassConstRefNonProviding));
static_assert(__is_convertible(DerivedRefProviding, ClassConstRefProviding));
static_assert(__is_nothrow_convertible(DerivedRefProviding,
                                       ClassConstRefProviding));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_convertible(Derived&, Base&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(Derived&, Base&));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedRefNonProviding, Base&));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedRefNonProviding, Base&));
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedRefProviding, Base&));
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedRefProviding, Base&));
// IWYU: Derived is...*-i2.h
static_assert(__is_convertible(DerivedPtrRefNonProviding,
                               ClassConstRefProviding));
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(DerivedPtrRefNonProviding,
                                       ClassConstRefProviding));
static_assert(__is_convertible(DerivedPtrRefProviding, ClassConstRefProviding));
static_assert(__is_nothrow_convertible(DerivedPtrRefProviding,
                                       ClassConstRefProviding));
// References to volatile don't bind to constructed temporary objects.
// IWYU: Derived needs a declaration
static_assert(!__is_convertible(Derived*, volatile Class&));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_convertible(Derived*, volatile Class&));
// IWYU: Derived needs a declaration
static_assert(!__is_convertible(Derived*, const volatile Class&));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_convertible(Derived*, const volatile Class&));
// IWYU: Derived needs a declaration
static_assert(!__is_convertible(Derived*, Class&));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_convertible(Derived*, Class&));
// Rvalue references bind to temporaries.
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Derived*, Class&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Derived*, Class&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Derived*, Class));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Derived*, Class));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Derived* const volatile&, Class&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Derived* const volatile&, Class&&));
// Unions don't take part in inheritance, so the pointed-to type in the Struct
// ctor should be exactly the same (Union1).
// IWYU: Struct is...*-i1.h
static_assert(__is_convertible(Union1* const volatile&, Struct&&));
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_convertible(Union1* const volatile&, Struct&&));
// IWYU: Struct is...*-i1.h
static_assert(__is_convertible(Union1PtrRefNonProviding, Struct&&));
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_convertible(Union1PtrRefNonProviding, Struct&&));
// Union1 has a constructor from 'const Base*'.
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_convertible(const Derived*, Union1&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_convertible(const Derived*, Union1&&));
static_assert(!__is_convertible(void, Class&&));
static_assert(!__is_nothrow_convertible(void, Class&&));
static_assert(!__is_convertible(Class&, void));
static_assert(!__is_nothrow_convertible(Class&, void));
// No conversion to function type.
static_assert(!__is_convertible(Class&, void()));
static_assert(!__is_nothrow_convertible(Class&, void()));
// But there may be conversions to function reference types.
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(Class&, void (&&)()));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(Class&, void (&&)()));
// The complete Class type is required according to the standard.
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible(Class, void()));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_convertible(Class, void()));
// IWYU: Class is...*-i1.h
static_assert(__is_convertible(void(), const Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_convertible(void(), const Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived*, Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived*, Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived[], Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived[], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived*, Base* const&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived*, Base* const&));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_convertible(Derived*, Base*&));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_convertible(Derived*, Base*&));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_convertible(Derived*&, Base*&));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_convertible(Derived*&, Base*&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived*&, Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived*&, Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedPtrRefNonProviding, Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedPtrRefNonProviding, Base*));
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedPtrRefProviding, Base*));
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedPtrRefProviding, Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedPtrNonProviding, Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedPtrNonProviding, Base*));
// IWYU: Base needs a declaration
static_assert(__is_convertible(DerivedPtrProviding, Base*));
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(DerivedPtrProviding, Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(int Base::*, int Derived::*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(int Base::*, int Derived::*));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_convertible(int Base::*, int Derived::*&));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_convertible(int Base::*, int Derived::*&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(int Base::*&, int Derived::*&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(int Base::*&, int Derived::*&&));
// IWYU: Derived is...*-i2.h
static_assert(__is_convertible(BaseMemPtr<int>, DerivedMemPtr<int>));
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(BaseMemPtr<int>, DerivedMemPtr<int>));
// IWYU: Derived is...*-i2.h
static_assert(__is_convertible(BaseMemPtr<int> volatile&,
                               DerivedMemPtr<int> const&));
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(BaseMemPtr<int> volatile&,
                                       DerivedMemPtr<int> const&));
static_assert(!__is_convertible(BaseMemPtr<int> const&,
                                DerivedMemPtr<int> volatile&));
static_assert(!__is_nothrow_convertible(BaseMemPtr<int> const&,
                                        DerivedMemPtr<int> volatile&));

// IWYU: Class is...*-i1.h
static_assert(__is_assignable(Class, Class));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_assignable(Class, Class));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_assignable(Class, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_assignable(Class*, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_assignable(Class*, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_assignable(Class*, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_assignable(Class, Class*));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_assignable(Class, Class*));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_assignable(Class, Class*));
static_assert(!__is_assignable(Class*, Class*));
static_assert(!__is_trivially_assignable(Class*, Class*));
static_assert(!__is_nothrow_assignable(Class*, Class*));
// TODO: no need of full type for arrays of unknown bound.
// IWYU: Class is...*-i1.h
static_assert(!__is_assignable(Class[], Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_assignable(Class[], Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_assignable(Class[], Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_assignable(Class[5], Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_assignable(Class[5], Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_assignable(Class[5], Class[5]));
// Returns true because Class has an operator=(int).
// IWYU: Class is...*-i1.h
static_assert(__is_assignable(Class&, int));
static_assert(!__is_trivially_assignable(Class&, int));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_assignable(Class&, int));
// Returns true because Class has an operator int().
// IWYU: Class is...*-i1.h
static_assert(__is_assignable(int&, Class&));
static_assert(!__is_trivially_assignable(int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_assignable(int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_assignable(int&, ClassRefNonProviding));
static_assert(!__is_trivially_assignable(int&, ClassRefNonProviding));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_assignable(int&, ClassRefNonProviding));
static_assert(__is_assignable(int&, ClassRefProviding));
static_assert(!__is_trivially_assignable(int&, ClassRefProviding));
static_assert(__is_nothrow_assignable(int&, ClassRefProviding));
static_assert(!__is_assignable(const int&, Class&));
static_assert(!__is_trivially_assignable(const int&, Class&));
static_assert(!__is_nothrow_assignable(const int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_assignable(volatile int&, Class&));
static_assert(!__is_trivially_assignable(volatile int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_assignable(volatile int&, Class&));
static_assert(!__is_assignable(const volatile int&, Class&));
static_assert(!__is_trivially_assignable(const volatile int&, Class&));
static_assert(!__is_nothrow_assignable(const volatile int&, Class&));
static_assert(!__is_assignable(int, Class&));
static_assert(!__is_trivially_assignable(int, Class&));
static_assert(!__is_nothrow_assignable(int, Class&));
static_assert(__is_assignable(Class*&, Class*));
static_assert(__is_trivially_assignable(Class*&, Class*));
static_assert(__is_nothrow_assignable(Class*&, Class*));
static_assert(__is_assignable(Class*&, Class*&));
static_assert(__is_trivially_assignable(Class*&, Class*&));
static_assert(__is_nothrow_assignable(Class*&, Class*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Base*&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(Base*&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Base*&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(const Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(const Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(const Base*&, const Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(const Base*&, volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(const Base*&, volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(const Base*&, volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(const volatile Base*&, volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(const volatile Base*&,
                                        // IWYU: Derived needs a declaration
                                        volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(const volatile Base*&,
                                      // IWYU: Derived needs a declaration
                                      volatile Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(Base* const&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Base* const&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(Base* const&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Base*&, Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(Base*&, Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Base*&, Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Base*&, DerivedPtrRefNonProviding));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(Base*&, DerivedPtrRefNonProviding));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Base*&, DerivedPtrRefNonProviding));
// IWYU: Base needs a declaration
static_assert(__is_assignable(Base*&, DerivedPtrRefProviding));
// IWYU: Base needs a declaration
static_assert(__is_trivially_assignable(Base*&, DerivedPtrRefProviding));
// IWYU: Base needs a declaration
static_assert(__is_nothrow_assignable(Base*&, DerivedPtrRefProviding));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(Base**&, Derived**));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Base**&, Derived**));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(Base**&, Derived**));
// Derived is required because Class allows assignment from Base.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class&, Derived*));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Class&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class&, Derived[]));
// TODO: no need of full type here.
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(!__is_trivially_assignable(Class&, Derived[]));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class&, Derived[]));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class, Derived&));
// Derived& could be trivially assignable to Class if it were inherited
// from Class.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(!__is_trivially_assignable(Class, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class&, Derived&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(!__is_trivially_assignable(Class&, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class&, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class&&, Derived&&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(!__is_trivially_assignable(Class&&, Derived&&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Class&&, Derived&&));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(ClassRefNonProviding, DerivedRefNonProviding));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(!__is_trivially_assignable(ClassRefNonProviding,
                                         DerivedRefNonProviding));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(ClassRefNonProviding,
                                      DerivedRefNonProviding));
static_assert(__is_assignable(ClassRefProviding, DerivedRefProviding));
static_assert(!__is_trivially_assignable(ClassRefProviding,
                                         DerivedRefProviding));
static_assert(__is_nothrow_assignable(ClassRefProviding, DerivedRefProviding));
// IWYU: Struct is...*-i1.h
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_assignable(Struct&, const StructDerivedClass&));
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_assignable(Struct&, const StructDerivedClass&));
// IWYU: Struct is...*-i1.h
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_nothrow_assignable(Struct&, const StructDerivedClass&));
// IWYU: Base needs a declaration
// IWYU: Base is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_assignable(Base&&, Struct&&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_assignable(Base&&, Struct&&));
// IWYU: Base needs a declaration
// IWYU: Base is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_assignable(Base&&, Struct&&));
// IWYU: Union1 is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_assignable(Union1&, Struct&));
// Unions cannot take part in inheritance, therefore they cannot be trivially
// assigned to/from other types.
static_assert(!__is_trivially_assignable(Union1&, Struct&));
// IWYU: Union1 is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_assignable(Union1&, Struct&));
// IWYU: Union1 is...*-i1.h
// IWYU: Union2 is...*-i1.h
static_assert(__is_assignable(Union2&, Union1&));
static_assert(!__is_trivially_assignable(Union2&, Union1&));
// IWYU: Union1 is...*-i1.h
// IWYU: Union2 is...*-i1.h
static_assert(__is_nothrow_assignable(Union2&, Union1&));
// IWYU: Struct is...*-i1.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Struct&, Union1&));
static_assert(!__is_trivially_assignable(Struct&, Union1&));
// IWYU: Struct is...*-i1.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Struct&, Union1&));
// Unions also might have no accessible operator=, hence the full type info is
// required even for __is_trivially_assignable.
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Union1&, Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_assignable(Union1&, Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Union1&, Union1&));
// IWYU: Struct is...*-i1.h
static_assert(__is_assignable(Struct&, Union1*));
static_assert(!__is_trivially_assignable(Struct&, Union1*));
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_assignable(Struct&, Union1*));
// IWYU: Struct is...*-i1.h
static_assert(__is_assignable(Struct&, Union1PtrRefNonProviding));
static_assert(!__is_trivially_assignable(Struct&, Union1PtrRefNonProviding));
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_assignable(Struct&, Union1PtrRefNonProviding));
// TODO: no need of the complete Union1 type for arrays of unknown bound.
// IWYU: Struct is...*-i1.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Struct&, Union1[]));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_assignable(Struct&, Union1[]));
// IWYU: Struct is...*-i1.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Struct&, Union1[]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Union1RefNonProviding, Union1RefNonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_assignable(Union1RefNonProviding,
                                        Union1RefNonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Union1RefNonProviding,
                                      Union1RefNonProviding));
static_assert(__is_assignable(Union1RefProviding, Union1RefProviding));
static_assert(__is_trivially_assignable(Union1RefProviding,
                                        Union1RefProviding));
static_assert(__is_nothrow_assignable(Union1RefProviding, Union1RefProviding));
static_assert(!__is_assignable(Class&, void));
static_assert(!__is_trivially_assignable(Class&, void));
static_assert(!__is_nothrow_assignable(Class&, void));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(int Derived::*&, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(int Derived::*&, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(int Derived::*&, int Base::*));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<int>&, BaseMemPtr<int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<int>&, BaseMemPtr<int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<int>&, BaseMemPtr<int>));
static_assert(!__is_assignable(BaseMemPtr<int>&, UnionMemPtr<int>));
static_assert(!__is_trivially_assignable(BaseMemPtr<int>&, UnionMemPtr<int>));
static_assert(!__is_nothrow_assignable(BaseMemPtr<int>&, UnionMemPtr<int>));
static_assert(__is_assignable(BaseMemPtr<int>&, BaseMemPtr<int>));
static_assert(__is_trivially_assignable(BaseMemPtr<int>&, BaseMemPtr<int>));
static_assert(__is_nothrow_assignable(BaseMemPtr<int>&, BaseMemPtr<int>));
static_assert(!__is_assignable(DerivedMemPtr<unsigned int>&, BaseMemPtr<int>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<unsigned int>&,
                                         BaseMemPtr<int>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<unsigned int>&,
                                       BaseMemPtr<int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<const int>&, BaseMemPtr<int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<const int>&,
                                        BaseMemPtr<int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<const int>&,
                                      BaseMemPtr<int>));
static_assert(!__is_assignable(DerivedMemPtr<const int>&,
                               BaseMemPtr<volatile int>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<const int>&,
                                         BaseMemPtr<volatile int>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<const int>&,
                                       BaseMemPtr<volatile int>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<int()>&, BaseMemPtr<int()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<int()>&,
                                        BaseMemPtr<int()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<int()>&,
                                      BaseMemPtr<int()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<int()>&,
                              BaseMemPtr<int() noexcept>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<int()>&,
                                        BaseMemPtr<int() noexcept>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<int()>&,
                                      BaseMemPtr<int() noexcept>));
static_assert(!__is_assignable(DerivedMemPtr<int() noexcept>&,
                               BaseMemPtr<int()>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<int() noexcept>&,
                                         BaseMemPtr<int()>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<int() noexcept>&,
                                       BaseMemPtr<int()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<int() noexcept>&,
                              BaseMemPtr<int() noexcept>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<int() noexcept>&,
                                        BaseMemPtr<int() noexcept>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<int() noexcept>&,
                                      BaseMemPtr<int() noexcept>));
static_assert(!__is_assignable(DerivedMemPtr<void()>&, BaseMemPtr<int()>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<void()>&,
                                         BaseMemPtr<int()>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<void()>&,
                                       BaseMemPtr<int()>));
static_assert(!__is_assignable(DerivedMemPtr<void()>&,
                               BaseMemPtr<int() noexcept>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<void()>&,
                                         BaseMemPtr<int() noexcept>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<void()>&,
                                       BaseMemPtr<int() noexcept>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(DerivedMemPtr<int (*)()>&,
                              BaseMemPtr<int (*)()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(DerivedMemPtr<int (*)()>&,
                                        BaseMemPtr<int (*)()>));
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(DerivedMemPtr<int (*)()>&,
                                      BaseMemPtr<int (*)()>));
static_assert(!__is_assignable(DerivedMemPtr<int (*)()>&,
                               BaseMemPtr<int (*)() noexcept>));
static_assert(!__is_trivially_assignable(DerivedMemPtr<int (*)()>&,
                                         BaseMemPtr<int (*)() noexcept>));
static_assert(!__is_nothrow_assignable(DerivedMemPtr<int (*)()>&,
                                       BaseMemPtr<int (*)() noexcept>));
// TODO: handle other types in an implicit conversion chain like Derived in
// __is_assignable(Base*&, Class&) when Class has operator Derived*().

// Types for is_layout_compatible trait should be complete types, cv void,
// or arrays of unknown bound (C++20 [meta.rel]).
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(Class, int));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(int, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(const Class, int));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(int, volatile Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(void, Class));
// IWYU: Class is...*-i1.h
static_assert(__is_layout_compatible(Class, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(Class[5], int));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(int, Class[5]));
// TODO: no need of full type for arrays of unknown bound.
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(Class[], int));
// IWYU: Class is...*-i1.h
static_assert(!__is_layout_compatible(int, Class[]));
static_assert(__is_layout_compatible(Class*, Class*));
static_assert(__is_layout_compatible(Class&, Class&));
static_assert(!__is_layout_compatible(Class*, Class&));
static_assert(!__is_layout_compatible(Class&, Class*));
static_assert(!__is_layout_compatible(Class*, Struct*));
static_assert(!__is_layout_compatible(Class&, Struct&));

// IWYU: Class is...*-i1.h
static_assert(!__reference_binds_to_temporary(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__reference_constructs_from_temporary(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__reference_converts_from_temporary(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__reference_binds_to_temporary(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__reference_constructs_from_temporary(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__reference_converts_from_temporary(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__reference_binds_to_temporary(void, Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__reference_constructs_from_temporary(void, Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__reference_converts_from_temporary(void, Class[5]));
// TODO: no need of full type for arrays of unknown bound.
// IWYU: Class is...*-i1.h
static_assert(!__reference_binds_to_temporary(void, Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__reference_constructs_from_temporary(void, Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__reference_converts_from_temporary(void, Class[]));
static_assert(!__reference_binds_to_temporary(Class&&, void));
static_assert(!__reference_constructs_from_temporary(Class&&, void));
static_assert(!__reference_converts_from_temporary(Class&&, void));
// Class has ctor from void() function type.
// IWYU: Class is...*-i1.h
static_assert(__reference_binds_to_temporary(Class&&, void()));
// IWYU: Class is...*-i1.h
static_assert(__reference_constructs_from_temporary(Class&&, void()));
// IWYU: Class is...*-i1.h
static_assert(__reference_converts_from_temporary(Class&&, void()));
// Class has the conversion function to void(&&)() reference-to-function type,
// but the reference binding cannot involve a temporary object.
static_assert(!__reference_binds_to_temporary(void (&&)(), Class&));
static_assert(!__reference_constructs_from_temporary(void (&&)(), Class&));
static_assert(!__reference_converts_from_temporary(void (&&)(), Class&));
// Class has the conversion function to int.
// IWYU: Class is...*-i1.h
static_assert(__reference_binds_to_temporary(int&&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__reference_constructs_from_temporary(int&&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__reference_converts_from_temporary(int&&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__reference_binds_to_temporary(const int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__reference_constructs_from_temporary(const int&, Class&));
// IWYU: Class is...*-i1.h
static_assert(__reference_converts_from_temporary(const int&, Class&));
static_assert(!__reference_binds_to_temporary(const volatile int&, Class&));
static_assert(!__reference_constructs_from_temporary(const volatile int&,
                                                     Class&));
static_assert(!__reference_converts_from_temporary(const volatile int&,
                                                   Class&));
static_assert(!__reference_binds_to_temporary(int, Class&));
static_assert(!__reference_constructs_from_temporary(int, Class&));
static_assert(!__reference_converts_from_temporary(int, Class&));
// Class has ctor from Base*.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_binds_to_temporary(Class&&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_constructs_from_temporary(Class&&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_converts_from_temporary(Class&&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__reference_binds_to_temporary(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__reference_constructs_from_temporary(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__reference_converts_from_temporary(Class, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__reference_binds_to_temporary(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__reference_constructs_from_temporary(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__reference_converts_from_temporary(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_binds_to_temporary(Base * &&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_constructs_from_temporary(Base * &&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_converts_from_temporary(Base * &&, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__reference_binds_to_temporary(int Derived::*, int Base::*));
static_assert(
    // IWYU: Derived needs a declaration
    !__reference_constructs_from_temporary(int Derived::*,
                                           // IWYU: Base needs a declaration
                                           int Base::*));
static_assert(
    // IWYU: Derived needs a declaration
    !__reference_converts_from_temporary(int Derived::*,
                                         // IWYU: Base needs a declaration
                                         int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__reference_binds_to_temporary(int Derived::*&&, int Base::*));
static_assert(
    // IWYU: Derived needs a declaration
    // IWYU: Derived is...*-i2.h
    __reference_constructs_from_temporary(int Derived::*&&,
                                          // IWYU: Base needs a declaration
                                          int Base::*));
static_assert(
    // IWYU: Derived needs a declaration
    // IWYU: Derived is...*-i2.h
    __reference_converts_from_temporary(int Derived::*&&,
                                        // IWYU: Base needs a declaration
                                        int Base::*));

/**** IWYU_SUMMARY

tests/cxx/type_trait.cc should add these lines:
#include "tests/cxx/type_trait-i1.h"
#include "tests/cxx/type_trait-i2.h"

tests/cxx/type_trait.cc should remove these lines:
- class Class;  // lines XX-XX
- class StructDerivedClass;  // lines XX-XX
- struct Struct;  // lines XX-XX
- union Union1;  // lines XX-XX
- union Union2;  // lines XX-XX

The full include-list for tests/cxx/type_trait.cc:
#include "tests/cxx/type_trait-d1.h"  // for ClassConstRefProviding, ClassRefProviding, DerivedPtrProviding, DerivedPtrRefProviding, DerivedRefProviding, Union1RefProviding
#include "tests/cxx/type_trait-d2.h"  // for BaseMemPtr, ClassConstRefNonProviding, ClassNonProviding, ClassRefNonProviding, DerivedMemPtr, DerivedPtrNonProviding, DerivedPtrRefNonProviding, DerivedRefNonProviding, Union1PtrRefNonProviding, Union1RefNonProviding, UnionMemPtr
#include "tests/cxx/type_trait-i1.h"  // for Base, Class, Struct, StructDerivedClass, Union1, Union2
#include "tests/cxx/type_trait-i2.h"  // for Derived

***** IWYU_SUMMARY */

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

// IWYU: Base is...*type_trait-i1.h
// IWYU: Base needs a declaration
// IWYU: Derived is...*type_trait-i2.h
// IWYU: Derived needs a declaration
static_assert(__is_convertible_to(Derived*, Base*),
              "Derived should be convertible to the Base class");

// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_convertible_to(Derived**, Base**),
              "Indirect pointers shouldn't be convertible");

// IWYU: Base is...*type_trait-i1.h
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
// IWYU: Derived needs a declaration
static_assert(__is_convertible_to(Derived&, Base&),
              "Derived should be convertible to the Base class");

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
static_assert(!__is_trivially_assignable(Union2&, Union1&));
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
// TODO: the full Base type is redundant here.
// IWYU: Base is...*tests/cxx/type_trait-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(int Derived::*&, int Base::*));
// IWYU: Base is...*tests/cxx/type_trait-i1.h
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(int Derived::*&, int Base::*));
// IWYU: Base is...*tests/cxx/type_trait-i1.h
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
#include "tests/cxx/type_trait-d1.h"  // for ClassRefProviding, DerivedPtrRefProviding, DerivedRefProviding, Union1RefProviding
#include "tests/cxx/type_trait-d2.h"  // for BaseMemPtr, ClassRefNonProviding, DerivedMemPtr, DerivedPtrRefNonProviding, DerivedRefNonProviding, Union1RefNonProviding, UnionMemPtr
#include "tests/cxx/type_trait-i1.h"  // for Base, Class, Struct, StructDerivedClass, Union1, Union2
#include "tests/cxx/type_trait-i2.h"  // for Derived

***** IWYU_SUMMARY */

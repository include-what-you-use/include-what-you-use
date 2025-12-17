//===--- type_trait.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20 -Wno-deprecated-builtins -fms-extensions

#include "tests/cxx/type_trait-d1.h"
#include "tests/cxx/type_trait-d2.h"

// To avoid lots of "needs a declaration" warnings.
class Class;
class StructDerivedClass;
class With3WayComp;
struct Struct;
union Union1;
union Union2;

using Void = void;
enum class ScopedEnum {};

constexpr bool complete = __is_complete_type(Class);
static_assert(!__is_void(Class));
static_assert(!__is_integral(Class));
static_assert(!__is_floating_point(Class));
static_assert(!__is_array(Class));
static_assert(__is_array(Class[]));
static_assert(!__is_bounded_array(Class));
static_assert(__is_bounded_array(Class[5]));
static_assert(!__is_pointer(Class));
static_assert(__is_pointer(Class*));
static_assert(!__is_lvalue_reference(Class));
static_assert(__is_lvalue_reference(Class&));
static_assert(!__is_rvalue_reference(Class));
static_assert(__is_rvalue_reference(Class&&));
static_assert(!__is_member_function_pointer(Class));
static_assert(__is_member_function_pointer(int (Class::*)()));
static_assert(!__is_member_object_pointer(Class));
static_assert(__is_member_object_pointer(int Class::*));
static_assert(!__is_enum(Class));
static_assert(!__is_scoped_enum(Class));
static_assert(!__is_union(Class));
static_assert(__is_union(Union1));
static_assert(__is_class(Class));
static_assert(!__is_function(Class));
static_assert(!__is_reference(Class));
static_assert(__is_reference(Class&));
static_assert(!__is_arithmetic(Class));
static_assert(!__is_fundamental(Class));
static_assert(__is_object(Class));
static_assert(!__is_scalar(Class));
static_assert(__is_compound(Class));
static_assert(!__is_member_pointer(Class));
static_assert(__is_member_pointer(Struct Class::*));
static_assert(!__is_const(Class));
static_assert(__is_const(const Class));
static_assert(!__is_volatile(Class));
static_assert(__is_volatile(volatile Class));
static_assert(!__is_signed(Class));
static_assert(!__is_unsigned(Class));
static_assert(!__is_unbounded_array(Class));
static_assert(__is_unbounded_array(Class[]));
static_assert(!__is_interface_class(Class));
// IWYU: Class is...*-i1.h
static_assert(__builtin_structured_binding_size(Class) == 2);
// IWYU: Class is...*-i1.h
static_assert(__builtin_structured_binding_size(ClassNonProviding) == 2);
static_assert(__builtin_structured_binding_size(ClassProviding) == 2);
static_assert(__builtin_structured_binding_size(Class[3]) == 3);
static_assert(__builtin_structured_binding_size(ClassArray3NonProviding) == 3);
// IWYU: Struct is...*-i1.h
static_assert(__is_empty(Struct));
static_assert(!__is_empty(Struct*));
static_assert(!__is_empty(Struct&));
static_assert(!__is_empty(Struct[]));
static_assert(!__is_empty(Struct[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_empty(ClassNonProviding));
static_assert(!__is_empty(ClassProviding));
static_assert(!__is_empty(Union1));
// IWYU: Class is...*-i1.h
static_assert(!__is_polymorphic(Class));
static_assert(!__is_polymorphic(Class*));
static_assert(!__is_polymorphic(Class&));
static_assert(!__is_polymorphic(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_polymorphic(ClassNonProviding));
static_assert(!__is_polymorphic(ClassProviding));
static_assert(!__is_polymorphic(Union1));
// IWYU: Class is...*-i1.h
static_assert(!__is_abstract(Class));
static_assert(!__is_abstract(Class*));
static_assert(!__is_abstract(Class&));
static_assert(!__is_abstract(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_abstract(ClassNonProviding));
static_assert(!__is_abstract(ClassProviding));
static_assert(!__is_abstract(Union1));
// IWYU: Class is...*-i1.h
static_assert(!__is_final(Class));
static_assert(!__is_final(Class*));
static_assert(!__is_final(Class&));
static_assert(!__is_final(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_final(ClassNonProviding));
static_assert(!__is_final(ClassProviding));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_final(Union1));
static_assert(!__is_final(Union1*));
static_assert(!__is_final(Union1&));
static_assert(!__is_final(Union1[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_sealed(Class));
static_assert(!__is_sealed(Class*));
static_assert(!__is_sealed(Class&));
static_assert(!__is_sealed(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_sealed(ClassNonProviding));
static_assert(!__is_sealed(ClassProviding));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_sealed(Union1));
static_assert(!__is_sealed(Union1*));
static_assert(!__is_sealed(Union1&));
static_assert(!__is_sealed(Union1[]));
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_aggregate(StructDerivedClass));
static_assert(!__is_aggregate(StructDerivedClass*));
static_assert(!__is_aggregate(StructDerivedClass&));
static_assert(__is_aggregate(Class[]));
static_assert(__is_aggregate(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_aggregate(ClassNonProviding));
static_assert(!__is_aggregate(ClassProviding));
static_assert(__is_aggregate(ClassArray2NonProviding));
// IWYU: Union2 is...*-i1.h
static_assert(__is_aggregate(Union2));
static_assert(!__is_aggregate(Union2*));
static_assert(!__is_aggregate(Union2&));
static_assert(__is_aggregate(Union1[]));
// IWYU: Class is...*-i1.h
static_assert(__builtin_is_implicit_lifetime(Class));
static_assert(__builtin_is_implicit_lifetime(Class*));
static_assert(!__builtin_is_implicit_lifetime(Class&));
static_assert(__builtin_is_implicit_lifetime(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__builtin_is_implicit_lifetime(ClassNonProviding));
static_assert(__builtin_is_implicit_lifetime(ClassProviding));
static_assert(__builtin_is_implicit_lifetime(ClassArray2NonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__builtin_is_implicit_lifetime(Union1));
static_assert(__builtin_is_implicit_lifetime(Union1*));
static_assert(!__builtin_is_implicit_lifetime(Union1&));
static_assert(__builtin_is_implicit_lifetime(Union1[]));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(Class));
static_assert(__has_unique_object_representations(Class*));
static_assert(!__has_unique_object_representations(Class&));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(Class[][5]));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(Class[5][5]));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(ClassNonProviding));
static_assert(__has_unique_object_representations(ClassProviding));
// IWYU: Class is...*-i1.h
static_assert(__has_unique_object_representations(ClassArray2NonProviding));
// IWYU: Derived is...*-i2.h
static_assert(!__has_unique_object_representations(DerivedArrayNonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_unique_object_representations(Union1));
static_assert(!__has_unique_object_representations(Union1&&));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_unique_object_representations(Union1[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(Class));
static_assert(__is_trivial(Class*));
static_assert(!__is_trivial(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(Class[5][6]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(Class[][6]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(ClassNonProviding));
static_assert(!__is_trivial(ClassProviding));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivial(ClassArray2NonProviding));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivial(DerivedArrayNonProviding));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivial(Union2));
static_assert(__is_trivial(Union1*));
static_assert(!__is_trivial(Union1&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivial(Union2[]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivial(Union2[2][3]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(Class));
static_assert(__is_trivially_copyable(Class*));
static_assert(!__is_trivially_copyable(Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(Class[5][6]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(Class[][6]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(ClassNonProviding));
static_assert(__is_trivially_copyable(ClassProviding));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_copyable(ClassArray2NonProviding));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_copyable(DerivedArrayNonProviding));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivially_copyable(Union2));
static_assert(__is_trivially_copyable(Union1*));
static_assert(!__is_trivially_copyable(Union1&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivially_copyable(Union2[]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivially_copyable(Union2[2][3]));
// IWYU: Class is...*-i1.h
static_assert(__is_standard_layout(Class));
static_assert(__is_standard_layout(Class*));
static_assert(!__is_standard_layout(Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_standard_layout(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__is_standard_layout(Class[5][6]));
// IWYU: Class is...*-i1.h
static_assert(__is_standard_layout(Class[][6]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_standard_layout(Union2));
static_assert(!__is_standard_layout(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_standard_layout(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_pod(Class));
static_assert(__is_pod(Class*));
static_assert(!__is_pod(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__is_pod(Class[][6]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_pod(Union2));
static_assert(!__is_pod(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_pod(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_literal(Class));
static_assert(__is_literal(Class*));
static_assert(__is_literal(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__is_literal(Class[][6]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_literal(Union2));
static_assert(__is_literal(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_literal(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__is_bitwise_cloneable(Class));
static_assert(__is_bitwise_cloneable(Class*));
static_assert(__is_bitwise_cloneable(Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_bitwise_cloneable(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_bitwise_cloneable(Union2));
static_assert(__is_bitwise_cloneable(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_bitwise_cloneable(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_relocatable(Class));
static_assert(__is_trivially_relocatable(Class*));
static_assert(!__is_trivially_relocatable(Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_relocatable(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivially_relocatable(Union2));
static_assert(!__is_trivially_relocatable(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__is_trivially_relocatable(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_equality_comparable(Class));
static_assert(__is_trivially_equality_comparable(Class*));
static_assert(!__is_trivially_equality_comparable(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_equality_comparable(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(!__is_trivially_equality_comparable(Union2));
static_assert(!__is_trivially_equality_comparable(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(!__is_trivially_equality_comparable(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__builtin_is_cpp_trivially_relocatable(Class));
static_assert(__builtin_is_cpp_trivially_relocatable(Class*));
static_assert(!__builtin_is_cpp_trivially_relocatable(Class&));
// IWYU: Class is...*-i1.h
static_assert(__builtin_is_cpp_trivially_relocatable(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__builtin_is_cpp_trivially_relocatable(Union2));
static_assert(!__builtin_is_cpp_trivially_relocatable(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__builtin_is_cpp_trivially_relocatable(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__can_pass_in_regs(Class));
// IWYU: Union1 is...*-i1.h
static_assert(__can_pass_in_regs(Union1));
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__has_nothrow_constructor(StructDerivedClass));
static_assert(__has_nothrow_constructor(Class*));
static_assert(!__has_nothrow_constructor(Class&));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_constructor(StructDerivedClass[5]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_constructor(Union2));
static_assert(!__has_nothrow_constructor(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_constructor(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_copy(Class));
static_assert(__has_nothrow_copy(Class*));
static_assert(__has_nothrow_copy(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_copy(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_copy(Union2));
static_assert(__has_nothrow_copy(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_copy(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_constructor(Class));
static_assert(__has_trivial_constructor(Class*));
static_assert(!__has_trivial_constructor(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_constructor(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_constructor(Union2));
static_assert(!__has_trivial_constructor(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_constructor(Union2[1]));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_move_constructor(Class));
static_assert(__has_trivial_move_constructor(Class*));
static_assert(!__has_trivial_move_constructor(Class&));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_move_constructor(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_move_constructor(Union2));
static_assert(!__has_trivial_move_constructor(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_move_constructor(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_copy(Class));
static_assert(__has_trivial_copy(Class*));
static_assert(__has_trivial_copy(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_copy(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_copy(Union2));
static_assert(__has_trivial_copy(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_copy(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_destructor(Class));
static_assert(__has_trivial_destructor(Class*));
static_assert(__has_trivial_destructor(Class&));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_destructor(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_destructor(Union2));
static_assert(__has_trivial_destructor(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_trivial_destructor(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_virtual_destructor(Class));
static_assert(!__has_virtual_destructor(Class*));
static_assert(!__has_virtual_destructor(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_virtual_destructor(Class[]));
// IWYU: Union2 is...*-i1.h
static_assert(!__has_virtual_destructor(Union2));
static_assert(!__has_virtual_destructor(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(!__has_virtual_destructor(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_assign(Class));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_assign(ClassNonProviding));
static_assert(__has_nothrow_assign(ClassProviding));
static_assert(__has_nothrow_assign(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(ClassRefNonProviding));
static_assert(!__has_nothrow_assign(ClassRefProviding));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(Class&&));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(Class (&)[5]));
static_assert(!__has_nothrow_assign(Class (&)[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(Class (&&)[5]));
static_assert(!__has_nothrow_assign(Class (&&)[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_assign(ClassArray3NonProviding&));
// Reference to an unbounded array type.
static_assert(!__has_nothrow_assign(DerivedArrayNonProviding&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_assign(Union2));
// IWYU: Union2 is...*-i1.h
static_assert(!__has_nothrow_assign(Union2&));
static_assert(__has_nothrow_assign(Union2*));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_assign(Union2[]));
// IWYU: Union2 is...*-i1.h
static_assert(!__has_nothrow_assign(Union2 (&)[5]));
static_assert(!__has_nothrow_assign(Union2 (&)[]));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_move_assign(Class));
static_assert(__has_nothrow_move_assign(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_move_assign(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_move_assign(Class&&));
// IWYU: Class is...*-i1.h
static_assert(__has_nothrow_move_assign(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_nothrow_move_assign(Class (&)[5]));
static_assert(!__has_nothrow_move_assign(Class (&)[]));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_move_assign(Union2));
// IWYU: Union2 is...*-i1.h
static_assert(!__has_nothrow_move_assign(Union2&));
// IWYU: Union2 is...*-i1.h
static_assert(__has_nothrow_move_assign(Union2[]));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_assign(Class));
static_assert(__has_trivial_assign(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_assign(Class&));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_assign(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_assign(Class (&&)[5]));
static_assert(!__has_trivial_assign(Class (&&)[]));
// IWYU: Union1 is...*-i1.h
static_assert(__has_trivial_assign(Union1));
static_assert(__has_trivial_assign(Union1*));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_trivial_assign(Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_trivial_assign(Union1 (&)[5]));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_move_assign(Class));
static_assert(__has_trivial_move_assign(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_move_assign(Class&));
// IWYU: Class is...*-i1.h
static_assert(__has_trivial_move_assign(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__has_trivial_move_assign(Class (&)[5]));
static_assert(!__has_trivial_move_assign(Class (&)[]));
// IWYU: Union1 is...*-i1.h
static_assert(__has_trivial_move_assign(Union1));
static_assert(__has_trivial_move_assign(Union1*));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_trivial_move_assign(Union1&&));
// IWYU: Union1 is...*-i1.h
static_assert(!__has_trivial_move_assign(Union1 (&&)[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_destructible(Class));
// IWYU: Class is...*-i1.h
static_assert(__is_destructible(ClassNonProviding));
static_assert(__is_destructible(ClassProviding));
static_assert(__is_destructible(Class*));
static_assert(__is_destructible(Class&));
static_assert(__is_destructible(Class&&));
static_assert(__is_destructible(ClassRefNonProviding));
static_assert(!__is_destructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__is_destructible(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_destructible(Class[5][6]));
static_assert(__is_destructible(Class (&)[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_destructible(ClassArray3NonProviding));
// Array of unknown bound.
static_assert(!__is_destructible(DerivedArrayNonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__is_destructible(Union1));
static_assert(__is_destructible(Union1&));
static_assert(!__is_destructible(Union1[]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_destructible(Union1[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_destructible(Class));
static_assert(__is_nothrow_destructible(Class*));
static_assert(__is_nothrow_destructible(Class&));
static_assert(!__is_nothrow_destructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_destructible(Class[5]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_destructible(Union1));
static_assert(__is_nothrow_destructible(Union1&));
static_assert(!__is_nothrow_destructible(Union1[]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_destructible(Union1[5]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_destructible(Class));
static_assert(__is_trivially_destructible(Class*));
static_assert(__is_trivially_destructible(Class&));
static_assert(!__is_trivially_destructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(__is_trivially_destructible(Class[5]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_destructible(Union1));
static_assert(__is_trivially_destructible(Union1&));
static_assert(!__is_trivially_destructible(Union1[]));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_destructible(Union1[5]));

static_assert(__is_same(Class, Class));
static_assert(__is_same(Class&, Class&));
static_assert(__is_same(Class*, Class*));
static_assert(__is_same(Union1, Union1));
static_assert(!__is_same(Union1, Union2));
static_assert(!__is_same(Class, Union1));

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

// __is_convertible_to is a MS trait used in the MS STL to implement
// std::is_convertible. Despite MSVC requires only the rhs type for some reason,
// the C++ standard requires both for the trait.
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible_to(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_convertible_to(void, Class));

// Likewise, __is_convertible and __is_nothrow_convertible are used by libc++.
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
static_assert(!__is_convertible(int, Class[]));
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
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived[], Base*));
// IWYU: Derived is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived[], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived (&)[], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived (&)[], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(Derived (&)[5], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_nothrow_convertible(Derived (&)[5], Base*));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_convertible(volatile Derived (&)[5], Base*));
// IWYU: Derived needs a declaration
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_convertible(volatile Derived (&)[5], Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Base needs a declaration
static_assert(__is_convertible(volatile Derived (&)[5], volatile Base*));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(volatile Derived (&)[5],
                                       // IWYU: Base needs a declaration
                                       volatile Base*));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_convertible(DerivedArrayNonProviding&, Base*));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_convertible(DerivedArrayNonProviding&, Base*));
// IWYU: Base needs a declaration
static_assert(!__is_convertible(volatile DerivedArrayNonProviding&, Base*));
static_assert(!__is_nothrow_convertible(volatile DerivedArrayNonProviding&,
                                        // IWYU: Base needs a declaration
                                        Base*));
// IWYU: Derived needs a declaration
static_assert(!__is_convertible(Derived (&)[5], int));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_convertible(Derived (&)[5], int));
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
static_assert(!__is_assignable(Class[], Class[]));
static_assert(!__is_trivially_assignable(Class[], Class[]));
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
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Base*&, Derived (&)[5]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(Base*&, Derived (&)[5]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Base*&, Derived (&)[5]));
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(int&, Derived (&)[5]));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(int&, Derived (&)[5]));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(int&, Derived (&)[5]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Base*&, Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(Base*&, Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(Base*&, Derived (&)[]));
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(int&, Derived (&)[]));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(int&, Derived (&)[]));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(int&, Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_assignable(Base*&, const Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Base*&, const Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_assignable(Base*&, const Derived (&)[]));
// IWYU: Base needs a declaration
static_assert(!__is_assignable(Base*&, const DerivedArrayNonProviding&));
// IWYU: Base needs a declaration
static_assert(!__is_trivially_assignable(Base*&,
                                         const DerivedArrayNonProviding&));
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_assignable(Base*&,
                                       const DerivedArrayNonProviding&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(const volatile Base*&,
                              const DerivedArrayNonProviding&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_trivially_assignable(const volatile Base*&,
                                        const DerivedArrayNonProviding&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_nothrow_assignable(const volatile Base*&,
                                      const DerivedArrayNonProviding&));
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
// IWYU: Derived needs a declaration
// IWYU: Derived is...*tests/cxx/type_trait-i2.h
static_assert(__is_assignable(Class&, Derived[]));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_assignable(Class&, Derived[]));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
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
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Union1NonProviding&, Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_assignable(Union1NonProviding&, Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Union1NonProviding&, Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_assignable(Union1&, Union1NonProviding&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_trivially_assignable(Union1&, Union1NonProviding&));
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_assignable(Union1&, Union1NonProviding&));
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
static_assert(!__is_layout_compatible(Class[], int));
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
static_assert(!__reference_binds_to_temporary(void, Class[]));
static_assert(!__reference_constructs_from_temporary(void, Class[]));
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

// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class));
static_assert(!__is_constructible(Class&));
static_assert(!__is_nothrow_constructible(Class&));
static_assert(!__is_constructible(const Class&));
static_assert(!__is_nothrow_constructible(const Class&));
static_assert(!__is_constructible(Class&&));
static_assert(!__is_nothrow_constructible(Class&&));
static_assert(__is_constructible(Class*));
static_assert(__is_nothrow_constructible(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[5]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class[5]));
// TODO: no need of full type for arrays of unknown bound.
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(void, Class));
static_assert(!__is_constructible(const Class&, void));
static_assert(!__is_nothrow_constructible(const Class&, void));
// IWYU: Class is...*-i1.h
static_assert(__is_constructible(Class, void()));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_constructible(Class, void()));
// IWYU: Class is...*-i1.h
static_assert(__is_constructible(const Class&, void()));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_constructible(const Class&, void()));
static_assert(!__is_constructible(Class&, void()));
static_assert(!__is_nothrow_constructible(Class&, void()));
static_assert(!__is_constructible(void(), Class&));
static_assert(!__is_nothrow_constructible(void(), Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_constructible(void (&&)(), Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_constructible(void (&&)(), Class&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Class, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(const Class&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(const Class&, Derived*));
// IWYU: Derived needs a declaration
static_assert(!__is_constructible(Class&, Derived*));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_constructible(Class&, Derived*));
static_assert(!__is_constructible(Class&, const Class&));
static_assert(!__is_nothrow_constructible(Class&, const Class&));
static_assert(__is_constructible(const Class&, Class&));
static_assert(__is_nothrow_constructible(const Class&, Class&));
static_assert(__is_constructible(const Class&&, Class&&));
static_assert(__is_nothrow_constructible(const Class&&, Class&&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(const Class&, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(const Class&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Base&, Derived&));
static_assert(__is_constructible(const Class*, Class*));
static_assert(__is_nothrow_constructible(const Class*, Class*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(const Base*&&, const Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(const Base*&&, const Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(int Derived::*, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(int Derived::*, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(int Derived::*&&, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(int Derived::*&&, int Base::*));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class, void, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class, void, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(void, void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(void, void, Class));
// Class& converts to int, Derived* to Base*, and Union1& to double.
// IWYU: Struct is...*-i1.h
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_constructible(Struct, Class&, Derived*, Union1&));
// IWYU: Struct is...*-i1.h
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__is_nothrow_constructible(Struct, Class&, Derived*, Union1&));
// References are not constructed from more than one initializer expression.
// IWYU: Derived needs a declaration
static_assert(!__is_constructible(const Struct&, Class&, Derived*, Union1&));
static_assert(
    // IWYU: Derived needs a declaration
    !__is_nothrow_constructible(const Struct&, Class&, Derived*, Union1&));
// IWYU: Derived needs a declaration
static_assert(!__is_constructible(Struct&, Class&, Derived*, Union1&));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_constructible(Struct&, Class&, Derived*, Union1&));
// IWYU: Derived needs a declaration
static_assert(!__is_constructible(Struct*, Class&, Derived*, Union1&));
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_constructible(Struct*, Class&, Derived*, Union1&));
// No construction from void, hence no need of the complete Class type.
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(Struct, Class&, void));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(Struct, Class&, void));
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(Struct, Class&, Void));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(Struct, Class&, Void));
// No need of the Union1 complete type (as opposed to struct or class cases).
// IWYU: Struct is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Struct, void(), Union1*, Derived&));
// IWYU: Struct is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Struct, void(), Union1*, Derived&));
// 'const Derived*' converts to 'const volatile Base*'.
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Union1, int, const Derived*));
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Union1, int, const Derived*));
// Union1& converts to double, and Derived*& to Base*.
// IWYU: Class is...*-i1.h
// IWYU: Union1 is...*-i1.h
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(ClassNonProviding,
                                 Union1RefNonProviding,
                                 DerivedPtrRefNonProviding));
// IWYU: Class is...*-i1.h
// IWYU: Union1 is...*-i1.h
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(ClassNonProviding,
                                         Union1RefNonProviding,
                                         DerivedPtrRefNonProviding));
// IWYU: Class is...*-i1.h
static_assert(__is_constructible(ClassNonProviding,
                                 Union1RefProviding,
                                 DerivedPtrRefProviding));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_constructible(ClassNonProviding,
                                         Union1RefProviding,
                                         DerivedPtrRefProviding));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
static_assert(__is_constructible(Class[3], Derived*, void(), Struct&&));
static_assert(
    // IWYU: Class is...*-i1.h
    // IWYU: Derived needs a declaration
    // IWYU: Derived is...*-i2.h
    // IWYU: Struct is...*-i1.h
    __is_nothrow_constructible(Class[3], Derived*, void(), Struct&&));
static_assert(
    // IWYU: Class is...*-i1.h
    // IWYU: Derived needs a declaration
    // IWYU: Derived is...*-i2.h
    // IWYU: Struct is...*-i1.h
    __is_constructible(ClassArray3NonProviding, Derived*, void(), Struct&&));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_constructible(ClassArray3NonProviding,
                                         // IWYU: Derived needs a declaration
                                         Derived*,
                                         void(),
                                         Struct&&));
// Might be true if Class had the default constructor.
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(Class[5], void(), Struct&));
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(Class[5], void(), Struct&));
// Array size is too small.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_constructible(Class[2], Derived*, void(), Struct&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_nothrow_constructible(Class[2], Derived*, void(), Struct&));
static_assert(
    // IWYU: Class is...*-i1.h
    // IWYU: Derived needs a declaration
    !__is_constructible(ClassArray2NonProviding, Derived*, void(), Struct&));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(ClassArray2NonProviding,
                                          // IWYU: Derived needs a declaration
                                          Derived*,
                                          void(),
                                          Struct&));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[5], void, Struct&));
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(Class[5], void, Struct&));
// Always 'false' for multidimensional arrays regardless of what Struct is.
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[5][5], Struct&));
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[5][5], Struct&));
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_constructible(Class[1], Struct&));
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_constructible(Class[1], Struct&));
// Union1 cannot have a base, pointer to which a Class ctor may accept.
// IWYU: Class is...*-i1.h
// IWYU: Union2 is...*-i1.h
static_assert(!__is_constructible(Class[2], Union1*, Union2&));
// IWYU: Class is...*-i1.h
// IWYU: Union2 is...*-i1.h
static_assert(!__is_nothrow_constructible(Class[2], Union1*, Union2&));
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Union1[2], Derived*&, Derived*&));
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Union1[2], Derived*&, Derived*&));
// IWYU: Class is...*-i1.h
static_assert(__is_constructible(int[2], int, Class&));
// IWYU: Class is...*-i1.h
static_assert(__is_nothrow_constructible(int[2], int, Class&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
static_assert(__is_constructible(Base* [3], Derived*, Base*, Struct*&));
static_assert(
    // IWYU: Base needs a declaration
    // IWYU: Derived needs a declaration
    // IWYU: Derived is...*-i2.h
    // IWYU: Struct is...*-i1.h
    __is_nothrow_constructible(Base* [3], Derived*, Base*, Struct*&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_constructible(Base* [3], Base*&, Struct*&&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_constructible(Base* [3], Base*&, Struct*&&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_constructible(Base* [3], BaseNonProviding*&, Struct*&&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_nothrow_constructible(Base* [3],
                                         BaseNonProviding*&,
                                         Struct*&&));
// No implicit conversion from 'int' to pointer.
// IWYU: Base needs a declaration
static_assert(!__is_constructible(Base* [3], Struct*, int));
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_constructible(Base* [3], Struct*, int));
// 'int*' is unrelated to 'Base*'.
// IWYU: Base needs a declaration
static_assert(!__is_constructible(Base* [3], Struct*, int*));
// IWYU: Base needs a declaration
static_assert(!__is_nothrow_constructible(Base* [3], Struct*, int*));
// All object pointer types can be converted to void* implicitly.
static_assert(__is_constructible(void* [3], Struct*));
static_assert(__is_nothrow_constructible(void* [3], Struct*));
static_assert(!__is_constructible(void* [3], Struct*, int));
static_assert(!__is_nothrow_constructible(void* [3], Struct*, int));
static_assert(__is_constructible(void* [3], Struct*, int*));
static_assert(__is_nothrow_constructible(void* [3], Struct*, int*));
static_assert(!__is_constructible(void* [3], Struct&, int));
static_assert(!__is_nothrow_constructible(void* [3], Struct&, int));
// Struct might have a conversion function.
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(void* [3], Struct&, int*));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(void* [3], Struct&, int*));
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(void* [3], Struct&, int*&));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(void* [3], Struct&, int*&));
// Incompatible cv-qualification.
static_assert(!__is_constructible(void* [3], Struct&, const int*));
static_assert(!__is_nothrow_constructible(void* [3], Struct&, const int*));
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(const volatile void* [3],
                                  Struct&,
                                  const int*));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(const volatile void* [3],
                                          Struct&,
                                          const int*));
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(void* [3], Struct&, decltype(nullptr)));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(void* [3],
                                          Struct&,
                                          decltype(nullptr)));
// Arithmetic types are interconvertible.
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(int[3], Struct&, double));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(int[3], Struct&, double));
// Scoped enumerations are not implicitly convertible to other types.
static_assert(!__is_constructible(int[3], Struct&, ScopedEnum));
static_assert(!__is_nothrow_constructible(int[3], Struct&, ScopedEnum));
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(ScopedEnum[3], Struct&, ScopedEnum));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(ScopedEnum[3], Struct&, ScopedEnum));
// An array of Derived can be converted to Base* implicitly.
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(Base* [3], Derived (&)[]));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(Base* [3], Derived (&)[]));
// The rhs converts to the same type as the lhs array element type.
static_assert(__is_constructible(Struct* [3], Struct (&)[]));
static_assert(__is_nothrow_constructible(Struct* [3], Struct (&)[]));
// Any array can be implicitly converted to void*.
static_assert(__is_constructible(void* [3], Struct (&)[]));
static_assert(__is_nothrow_constructible(void* [3], Struct (&)[]));
// Struct may have 'operator Struct*()'.
// IWYU: Struct is...*-i1.h
static_assert(!__is_constructible(Struct* [3], Struct&));
// IWYU: Struct is...*-i1.h
static_assert(!__is_nothrow_constructible(Struct* [3], Struct&));
// Class may have a conversion function to e.g. 'int Base::*'.
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(int Derived::* [3], int Base::*, Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(int Derived::* [3],
                                          // IWYU: Base needs a declaration
                                          int Base::*,
                                          Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(int Derived::* [3], Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(int Derived::* [3], Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(int Derived::* [3], int Derived::*, Class&));
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Class is...*-i1.h
static_assert(!__is_nothrow_constructible(int Derived::* [3],
                                          // IWYU: Derived needs a declaration
                                          int Derived::*,
                                          Class&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(int Derived::* [3], int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(int Derived::* [3], int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(int Derived::* [3], int Base::*&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(int Derived::* [3], int Base::*&));
// IWYU: Base needs a declaration
static_assert(__is_constructible(int DerivedProviding::* [3], int Base::*));
static_assert(__is_nothrow_constructible(int DerivedProviding::* [3],
                                         // IWYU: Base needs a declaration
                                         int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_constructible(int DerivedNonProviding::* [3], int Base::*));
// IWYU: Derived is...*-i2.h
static_assert(__is_nothrow_constructible(int DerivedNonProviding::* [3],
                                         // IWYU: Base needs a declaration
                                         int Base::*));
static_assert(__is_constructible(int Class::* [3], int Class::*));
static_assert(__is_nothrow_constructible(int Class::* [3], int Class::*));
static_assert(__is_constructible(int Class::* [3], int Class::*&));
static_assert(__is_nothrow_constructible(int Class::* [3], int Class::*&));
static_assert(__is_constructible(const int Class::* [3], int Class::*));
static_assert(__is_nothrow_constructible(const int Class::* [3], int Class::*));
static_assert(__is_constructible(int ClassNonProviding::* [3], int Class::*));
static_assert(__is_nothrow_constructible(int ClassNonProviding::* [3],
                                         int Class::*));
static_assert(__is_constructible(int Class::* [3], int ClassNonProviding::*));
static_assert(__is_nothrow_constructible(int Class::* [3],
                                         int ClassNonProviding::*));
static_assert(__is_constructible(const int Class::* [3]));
static_assert(__is_nothrow_constructible(const int Class::* [3]));
// Currently always 'false' for arrays of unknown bound, but there is an issue:
// https://wg21.link/lwg3486
// so it's better to require types.
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
// IWYU: Union1 is...*-i1.h
static_assert(!__is_constructible(Class[], Struct&, Union1&));
static_assert(!__is_constructible(Class[], Struct&, void));
static_assert(!__is_constructible(Class[][5], Struct&));
// TODO: could it be 'true'? GCC accepts 'new int[]();'.
// IWYU: Class is...*-i1.h
static_assert(!__is_constructible(Class[]));

// IWYU: Base is...*-i1.h
struct PrivatelyDerived : private Base {
  // Evaluation occurs in the global context, hence always 'false'.
  // IWYU: Base needs a declaration
  static_assert(!__is_constructible(Base* [3], PrivatelyDerived*, Struct&));
};

// Check that IWYU doesn't crash on invalid template instantiation location.
template <typename T>
struct Tpl {};
static_assert(!__is_constructible(Class* [5], Tpl<int>*));

// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class));
static_assert(!__is_trivially_constructible(Class&));
static_assert(!__is_trivially_constructible(const Class&));
static_assert(!__is_trivially_constructible(Class&&));
static_assert(__is_trivially_constructible(Class*));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class[5]));
static_assert(!__is_trivially_constructible(Class[]));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(void, Class));
static_assert(!__is_trivially_constructible(const Class&, void));
static_assert(!__is_trivially_constructible(void, const Class&));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, int));
// Even with aggregates in C++20 mode, references are not trivially
// constructible from unrelated types.
static_assert(!__is_trivially_constructible(const Class&, int));
static_assert(!__is_trivially_constructible(Class&, int));
static_assert(!__is_trivially_constructible(int, Class&));
static_assert(!__is_trivially_constructible(int&&, Class&));
// Might be 'true' if Class were:
// class Class { public: Base* pb; };
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Class, Derived*));
// Similarly, Class might be:
// class Class { public: Base& b; };
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Class, Derived&));
// Unions don't take part in inheritance, so the complete Union1 type is not
// needed even if the traits evaluated to true.
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, Union1*));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, Union1&));
// Similarly, might be 'true'.
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Union1, Derived*));
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Union1, Derived*&));
// Unions cannot contain reference members, but can contain Base by value.
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Union1, Derived&));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1, Union2*));
// The complete Class type is required despite the trait is guaranteed
// to be false.
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, void, void));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class, int, int));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(void, void, Class));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(int, int, Class));
// Might be true if Class were an aggregate with corresponding fields.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Class, int, Derived&));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Class, Union1&, Derived*));
// IWYU: Class is...*-i1.h
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(ClassNonProviding,
                                            Union1RefNonProviding,
                                            DerivedPtrRefNonProviding));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(ClassNonProviding,
                                            Union1RefNonProviding,
                                            DerivedPtrRefProviding));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
static_assert(!__is_trivially_constructible(Class, Derived*, int, Struct*));
// No initialization from void.
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Class, Derived*, void));
// IWYU: Class is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Class, Void, Derived*));
// Unions cannot have multiple items in the initializer.
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Union1, Derived*, int));
// IWYU: Union1 is...*-i1.h
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Union1NonProviding, Derived*, int));
// IWYU: Base is...*-i1.h
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(
    // IWYU: Base needs a declaration
    Base[3],
    // IWYU: Derived needs a declaration
    Derived&,
    const Struct&,
    StructDerivedClass&&));
// IWYU: Base is...*-i1.h
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(
    // IWYU: Base needs a declaration
    Base[5],
    // IWYU: Derived needs a declaration
    Derived&,
    const Struct&,
    StructDerivedClass&&));
// IWYU: Base is...*-i1.h
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(BaseNonProviding[5],
                                           DerivedNonProviding&));
// IWYU: Base is...*-i1.h
static_assert(__is_trivially_constructible(BaseNonProviding[5],
                                           DerivedProviding&));
// IWYU: Base is...*-i1.h
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(BaseNonProviding[5],
                                           DerivedRefNonProviding));
// IWYU: Base is...*-i1.h
static_assert(__is_trivially_constructible(BaseNonProviding[5],
                                           DerivedRefProviding));
// Array size is too small.
// IWYU: Base is...*-i1.h
static_assert(!__is_trivially_constructible(
    // IWYU: Base needs a declaration
    Base[2],
    // IWYU: Derived needs a declaration
    Derived&,
    const Struct&,
    StructDerivedClass&&));
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(
    ClassArray2NonProviding, Struct&, Struct&, Struct&));
// Might be true if Struct were derived from Class.
// IWYU: Class is...*-i1.h
// IWYU: Struct is...*-i1.h
static_assert(!__is_trivially_constructible(
    ClassArray3NonProviding, Struct&, Struct&, Struct&));
// The wording suggests the possibility of 'true' for arrays of unknown bound
// despite compilers currently evaluate to 'false'.
// See https://wg21.link/lwg3486
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: StructDerivedClass is...*-i1.h
static_assert(!__is_trivially_constructible(
    // IWYU: Base needs a declaration
    Base[],
    // IWYU: Derived needs a declaration
    Derived&,
    const Struct&,
    StructDerivedClass&&));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1[], Union1&, const Union1&));
static_assert(!__is_trivially_constructible(Union1[], Union1&, Union1*));
static_assert(!__is_trivially_constructible(Union1[], Union1&, Union2&));
static_assert(!__is_trivially_constructible(Union1[], Union1&, Class&));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1NonProviding[], Union1&));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1[], Union1NonProviding&));
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1[], Union1RefNonProviding));
// 'false' for multidimensional arrays.
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class[5][5], Struct&));
// No trivial conversion from int to the array element even in the case
// of aggregate classes.
// IWYU: Base needs a declaration
// IWYU: Base is...*-i1.h
static_assert(!__is_trivially_constructible(Base[5], Struct&, int));
// The same for pointers...
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class[5], Struct&, Struct*));
/// ... and unions (because Union1 cannot be derived from Class).
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(Class[5], Struct&, Union1&));
// Struct and Class cannot be derived from Union1.
// IWYU: Union1 is...*-i1.h
static_assert(!__is_trivially_constructible(Union1[2], Struct&, Class&));
// No trivial conversion from class type to int.
static_assert(!__is_trivially_constructible(int[2], Class&, Struct&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base* [5], Derived*, Struct*&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base* [5], DerivedPtrNonProviding));
// IWYU: Base needs a declaration
static_assert(__is_trivially_constructible(Base* [5], DerivedPtrProviding));
// No implicit conversion from int to pointer.
// IWYU: Base needs a declaration
static_assert(!__is_trivially_constructible(Base* [5], Struct*, int));
// nullptr is convertible to any object pointer type, hence true.
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base* [5],
                                           decltype(nullptr),
                                           Struct*));
// true, but no full type is needed.
static_assert(__is_trivially_constructible(Class* [5], Class*));
// The complete type is not needed for Base.
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base* [5], Base*&&, Struct*));
// Arrays decay to pointers.
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base* [5],
                                           // IWYU: Base needs a declaration
                                           Base (&)[3],
                                           Struct (&)[3]));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base* [5],
                                           BaseNonProviding (&)[3],
                                           Struct (&)[3]));
// All object pointers are convertible to void*, no full type needed.
static_assert(__is_trivially_constructible(void* [5], Class*, Struct*));
static_assert(__is_trivially_constructible(Void* [5], Class*, Struct*));
// Class cannot be trivially converted to a pointer.
static_assert(!__is_trivially_constructible(Struct* [5],
                                            StructDerivedClass*,
                                            Class&));
static_assert(!__is_trivially_constructible(Struct* [5],
                                            StructDerivedClass*,
                                            ClassNonProviding&));
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(int StructDerivedClass::* [5],
                                           int Struct::*&,
                                           // IWYU: Base needs a declaration
                                           int Base::*));
// true, but Class full type is not needed.
static_assert(__is_trivially_constructible(const int Class::* [5],
                                           int Class::*&,
                                           int Class::*));
// The same.
static_assert(__is_trivially_constructible(const int Class::* [5],
                                           int ClassNonProviding::*));
// The same.
static_assert(__is_trivially_constructible(int ClassNonProviding::* [5],
                                           int Class::*));
// The same.
static_assert(__is_trivially_constructible(int Class::* [5],
                                           decltype(nullptr)));
// nullptr is convertible to a pointer to member.
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(int StructDerivedClass::* [5],
                                           decltype(nullptr),
                                           int Struct::*));
static_assert(__is_trivially_constructible(int Class::* [5],
                                           decltype(nullptr),
                                           int Class::*));
// Object pointers are not convertible to member pointers.
static_assert(!__is_trivially_constructible(int StructDerivedClass::* [5],
                                            int Struct::*,
                                            int*));
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(int Struct::* [5],
                                           int Struct::*,
                                           // IWYU: Base needs a declaration
                                           int Base::*));
// Compatible cv-qualification.
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(const int StructDerivedClass::* [5],
                                           int Struct::*));
// Incompatible cv-qualification.
static_assert(!__is_trivially_constructible(int StructDerivedClass::* [5],
                                            const int Struct::*));
// Class cannot be trivially converted to a member pointer, hence
// StructDerivedClass type info is not needed. Class type is required
// to be complete.
// IWYU: Class is...*-i1.h
static_assert(!__is_trivially_constructible(int StructDerivedClass::* [5],
                                            int Struct::*,
                                            Class));
static_assert(__is_trivially_constructible(int DerivedProviding::* [3],
                                           // IWYU: Base needs a declaration
                                           int Base::*));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(int DerivedNonProviding::* [3],
                                           // IWYU: Base needs a declaration
                                           int Base::*));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(DerivedMemPtr<int>[3],
                                           BaseMemPtr<int>));
static_assert(__is_trivially_constructible(int Class::* [3]));
// Class cannot be trivially convertible to int.
static_assert(!__is_trivially_constructible(int[3], Class&));
// Class* is unrelated to int*.
static_assert(!__is_trivially_constructible(int* [3], Class*));
// References cannot have multiple items in the initializer.
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Class&&, Derived*, int));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const Base&, Derived&, Derived&));
// Pointers cannot have multiple items in the initializer.
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base*, Derived*, Derived*));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const Class&, Derived*));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Class&, Derived*));
static_assert(!__is_trivially_constructible(Class&, const Class&));
static_assert(__is_trivially_constructible(const Class&, Class&));
static_assert(__is_trivially_constructible(const Class&&, Class&&));
static_assert(!__is_trivially_constructible(Class&, Class&&));
static_assert(!__is_trivially_constructible(Class&&, Class&));
static_assert(__is_trivially_constructible(ClassNonProviding&, Class&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base&, DerivedRefNonProviding));
// IWYU: Base needs a declaration
static_assert(__is_trivially_constructible(Base&, DerivedRefProviding));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__is_trivially_constructible(Base&, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base&, const Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const Base&, const Derived&));
// IWYU: Base needs a declaration
static_assert(!__is_trivially_constructible(const Base&,
                                            // IWYU: Derived needs a declaration
                                            const volatile Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const volatile Base&,
                                           // IWYU: Derived needs a declaration
                                           const Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base&&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base&&, const Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const Base&&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const volatile Base&&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const Base&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const Base&, volatile Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const volatile Base&, Derived&&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base&&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Base&&, const Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const Base&&, Derived&));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Union1&, Derived&));
// IWYU: Base needs a declaration
static_assert(!__is_trivially_constructible(Base&, Union1&));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(Union1&&, Derived&&));
// IWYU: Base needs a declaration
static_assert(!__is_trivially_constructible(Base&&, Union1&&));
// IWYU: Derived needs a declaration
static_assert(!__is_trivially_constructible(const Union1&, Derived&&));
// IWYU: Base needs a declaration
static_assert(!__is_trivially_constructible(const Base&, Union1&&));
// IWYU: Base needs a declaration
// IWYU: Struct is...*-i1.h
static_assert(__is_trivially_constructible(Base&, Struct&));
// IWYU: StructDerivedClass is...*-i1.h
static_assert(__is_trivially_constructible(Struct&&, StructDerivedClass&&));
static_assert(__is_trivially_constructible(const Class*, Class*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(const Base*&&, const Derived*&));
// IWYU: Base needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(Base*, DerivedPtrRefNonProviding));
// IWYU: Base needs a declaration
static_assert(__is_trivially_constructible(Base*, DerivedPtrRefProviding));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(int Derived::*, int Base::*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(int Derived::*&&, int Base::*));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(DerivedMemPtr<int>,
                                           BaseMemPtr<int>));
// IWYU: Derived is...*-i2.h
static_assert(__is_trivially_constructible(DerivedMemPtr<int>&&,
                                           BaseMemPtr<int>&));

static_assert(!__is_base_of(int, Class));
static_assert(!__is_pointer_interconvertible_base_of(int, Class));
static_assert(!__builtin_is_virtual_base_of(int, Class));
static_assert(!__is_base_of(Class, int));
static_assert(!__is_pointer_interconvertible_base_of(Class, int));
static_assert(!__builtin_is_virtual_base_of(Class, int));
static_assert(__is_base_of(Class, Class));
static_assert(__is_pointer_interconvertible_base_of(Class, Class));
// IWYU: Class is...*-i1.h
static_assert(!__builtin_is_virtual_base_of(Class, Class));
static_assert(__is_base_of(Class, ClassNonProviding));
static_assert(__is_pointer_interconvertible_base_of(Class, ClassNonProviding));
// IWYU: Class is...*-i1.h
static_assert(!__builtin_is_virtual_base_of(Class, ClassNonProviding));
static_assert(__is_base_of(ClassNonProviding, Class));
static_assert(__is_pointer_interconvertible_base_of(ClassNonProviding, Class));
// IWYU: Class is...*-i1.h
static_assert(!__builtin_is_virtual_base_of(ClassNonProviding, Class));
static_assert(__is_base_of(Struct, Struct));
static_assert(__is_pointer_interconvertible_base_of(Struct, Struct));
// IWYU: Struct is...*-i1.h
static_assert(!__builtin_is_virtual_base_of(Struct, Struct));
static_assert(!__is_base_of(Union1, Union1));
static_assert(!__is_pointer_interconvertible_base_of(Union1, Union1));
static_assert(!__builtin_is_virtual_base_of(Union1, Union1));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_base_of(Base, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__is_pointer_interconvertible_base_of(Base, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(!__builtin_is_virtual_base_of(Base, Derived));
// IWYU: Derived is...*-i2.h
static_assert(__is_base_of(BaseNonProviding, DerivedNonProviding));
// IWYU: Derived is...*-i2.h
static_assert(__is_pointer_interconvertible_base_of(BaseNonProviding,
                                                    DerivedNonProviding));
// IWYU: Derived is...*-i2.h
static_assert(!__builtin_is_virtual_base_of(BaseNonProviding,
                                            DerivedNonProviding));
static_assert(__is_base_of(BaseNonProviding, DerivedProviding));
static_assert(__is_pointer_interconvertible_base_of(BaseNonProviding,
                                                    DerivedProviding));
static_assert(!__builtin_is_virtual_base_of(BaseNonProviding,
                                            DerivedProviding));
// IWYU: Struct is...*-i1.h
static_assert(!__is_base_of(Class, Struct));
// IWYU: Struct is...*-i1.h
static_assert(!__is_pointer_interconvertible_base_of(Class, Struct));
// IWYU: Struct is...*-i1.h
static_assert(!__builtin_is_virtual_base_of(Class, Struct));
static_assert(!__is_base_of(Union1, Struct));
static_assert(!__is_pointer_interconvertible_base_of(Union1, Struct));
static_assert(!__builtin_is_virtual_base_of(Union1, Struct));
static_assert(!__is_base_of(Struct, Union1));
static_assert(!__is_pointer_interconvertible_base_of(Struct, Union1));
static_assert(!__builtin_is_virtual_base_of(Struct, Union1));
static_assert(!__is_base_of(Union1, Union2));
static_assert(!__is_pointer_interconvertible_base_of(Union1, Union2));
static_assert(!__builtin_is_virtual_base_of(Union1, Union2));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_base_of(Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_pointer_interconvertible_base_of(Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__builtin_is_virtual_base_of(Base&, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_base_of(Base&, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_pointer_interconvertible_base_of(Base&, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__builtin_is_virtual_base_of(Base&, Derived));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_base_of(Base, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_pointer_interconvertible_base_of(Base, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__builtin_is_virtual_base_of(Base, Derived&));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_base_of(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__is_pointer_interconvertible_base_of(Base*, Derived*));
// IWYU: Base needs a declaration
// IWYU: Derived needs a declaration
static_assert(!__builtin_is_virtual_base_of(Base*, Derived*));

template <typename>
struct DeducibleTpl;

// Check that the implicit "deducible" trait (introduced due to the alias
// template argument deduction on tpl_int definition below) does not trigger
// a requirement of complete DeducibleTpl here.
template <typename T>
using NonProvidingAliasTpl = DeducibleTpl<T>;

// IWYU: DeducibleTpl is...*-i1.h
NonProvidingAliasTpl tpl_int = 1;

// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(With3WayComp, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(With3WayComp, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(With3WayComp, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(int, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(int, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(int, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(With3WayComp&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(With3WayComp&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(With3WayComp&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(int, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(int, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(int, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp&&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(With3WayComp&&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(With3WayComp&&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(With3WayComp&&, int));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int, With3WayComp&&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_gt_synthesizes_from_spaceship(int, With3WayComp&&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_le_synthesizes_from_spaceship(int, With3WayComp&&));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_ge_synthesizes_from_spaceship(int, With3WayComp&&));
static_assert(!__builtin_lt_synthesizes_from_spaceship(With3WayComp*, int));
static_assert(!__builtin_gt_synthesizes_from_spaceship(With3WayComp*, int));
static_assert(!__builtin_le_synthesizes_from_spaceship(With3WayComp*, int));
static_assert(!__builtin_ge_synthesizes_from_spaceship(With3WayComp*, int));
static_assert(!__builtin_lt_synthesizes_from_spaceship(int, With3WayComp*));
static_assert(!__builtin_gt_synthesizes_from_spaceship(int, With3WayComp*));
static_assert(!__builtin_le_synthesizes_from_spaceship(int, With3WayComp*));
static_assert(!__builtin_ge_synthesizes_from_spaceship(int, With3WayComp*));
// No comparisons with void.
static_assert(!__builtin_lt_synthesizes_from_spaceship(With3WayComp, void));
static_assert(!__builtin_gt_synthesizes_from_spaceship(With3WayComp, void));
static_assert(!__builtin_le_synthesizes_from_spaceship(With3WayComp, void));
static_assert(!__builtin_ge_synthesizes_from_spaceship(With3WayComp, void));
static_assert(!__builtin_lt_synthesizes_from_spaceship(void, With3WayComp));
static_assert(!__builtin_gt_synthesizes_from_spaceship(void, With3WayComp));
static_assert(!__builtin_le_synthesizes_from_spaceship(void, With3WayComp));
static_assert(!__builtin_ge_synthesizes_from_spaceship(void, With3WayComp));
// With3WayComp has no comparison operator with int*, but might have.
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_lt_synthesizes_from_spaceship(With3WayComp&, int*));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_gt_synthesizes_from_spaceship(With3WayComp&, int*));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_le_synthesizes_from_spaceship(With3WayComp&, int*));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_ge_synthesizes_from_spaceship(With3WayComp&, int*));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_lt_synthesizes_from_spaceship(int*, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_gt_synthesizes_from_spaceship(int*, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_le_synthesizes_from_spaceship(int*, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
static_assert(!__builtin_ge_synthesizes_from_spaceship(int*, With3WayComp&));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, Class));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_gt_synthesizes_from_spaceship(With3WayComp, Class));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_le_synthesizes_from_spaceship(With3WayComp, Class));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_ge_synthesizes_from_spaceship(With3WayComp, Class));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Class, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_gt_synthesizes_from_spaceship(Class, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_le_synthesizes_from_spaceship(Class, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Class is...*-i1.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_ge_synthesizes_from_spaceship(Class, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator< is...*-i1.h
static_assert(!__builtin_lt_synthesizes_from_spaceship(With3WayComp, Struct));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator> is...*-i1.h
static_assert(!__builtin_gt_synthesizes_from_spaceship(With3WayComp, Struct));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator<= is...*-i1.h
static_assert(!__builtin_le_synthesizes_from_spaceship(With3WayComp, Struct));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator>= is...*-i1.h
static_assert(!__builtin_ge_synthesizes_from_spaceship(With3WayComp, Struct));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator< is...*-i1.h
static_assert(!__builtin_lt_synthesizes_from_spaceship(Struct, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator> is...*-i1.h
static_assert(!__builtin_gt_synthesizes_from_spaceship(Struct, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator<= is...*-i1.h
static_assert(!__builtin_le_synthesizes_from_spaceship(Struct, With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Struct is...*-i1.h
// IWYU: operator>= is...*-i1.h
static_assert(!__builtin_ge_synthesizes_from_spaceship(Struct, With3WayComp));

class LValueUsesNonMemberOp {};
class RValueUsesNonMemberOp {};

static_assert(!__builtin_lt_synthesizes_from_spaceship(int,
                                                       LValueUsesNonMemberOp));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int,
                                                      LValueUsesNonMemberOp&));
static_assert(
    !__builtin_lt_synthesizes_from_spaceship(int, LValueUsesNonMemberOp&&));
static_assert(!__builtin_lt_synthesizes_from_spaceship(LValueUsesNonMemberOp,
                                                       int));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(LValueUsesNonMemberOp&,
                                                      int));
static_assert(!__builtin_lt_synthesizes_from_spaceship(LValueUsesNonMemberOp&&,
                                                       int));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int,
                                                      RValueUsesNonMemberOp));
static_assert(!__builtin_lt_synthesizes_from_spaceship(int,
                                                       RValueUsesNonMemberOp&));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int,
                                                      RValueUsesNonMemberOp&&));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(RValueUsesNonMemberOp,
                                                      int));
static_assert(!__builtin_lt_synthesizes_from_spaceship(RValueUsesNonMemberOp&,
                                                       int));
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(RValueUsesNonMemberOp&&,
                                                      int));
// The complete Derived type is required for Derived* to Base* conversion.
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, Derived*));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Derived*, With3WayComp));
// Array-to-pointer conversions.
static_assert(
    // IWYU: With3WayComp is...*-i2.h
    // IWYU: Derived is...*-i2.h
    __builtin_lt_synthesizes_from_spaceship(With3WayComp,
                                            // IWYU: Derived needs a declaration
                                            Derived[5]));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Derived[5],
                                                      With3WayComp));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, Derived[]));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Derived[], With3WayComp));
static_assert(
    // IWYU: With3WayComp is...*-i2.h
    // IWYU: Derived is...*-i2.h
    __builtin_lt_synthesizes_from_spaceship(With3WayComp&,
                                            // IWYU: Derived needs a declaration
                                            Derived (&)[]));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Derived (&)[],
                                                      With3WayComp&));
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayCompRefProviding,
                                                      DerivedPtrNonProviding));
// IWYU: Derived is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(
    DerivedPtrNonProviding, With3WayCompRefProviding));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(
    With3WayCompRefNonProviding, DerivedPtrProviding));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(
    DerivedPtrProviding, With3WayCompRefNonProviding));
// No operator<=> for arrays.
static_assert(!__builtin_lt_synthesizes_from_spaceship(With3WayComp[5], int));
static_assert(!__builtin_lt_synthesizes_from_spaceship(int, With3WayComp[5]));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, int* [5]));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int* [5], With3WayComp));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, int**));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(int**, With3WayComp));
// IWYU: Enum needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Enum, Derived*));
// IWYU: Enum needs a declaration
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Derived*, Enum));
// Unions cannot have base classes, hence no full Union1 type is needed.
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, Union1*));
// IWYU: With3WayComp is...*-i2.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Union1*, With3WayComp));
// An implicit conversion sequence Union1 -> double -> int works here.
// IWYU: With3WayComp is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayComp, Union1&&));
// IWYU: With3WayComp is...*-i2.h
// IWYU: Union1 is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Union1&&, With3WayComp));
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayCompRefProviding,
                                                      Union1RefProviding));
static_assert(__builtin_lt_synthesizes_from_spaceship(
    Union1RefProviding, With3WayCompRefProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(With3WayCompRefProviding,
                                                      Union1RefNonProviding));
// IWYU: Union1 is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(
    Union1RefNonProviding, With3WayCompRefProviding));
// Both arg types should be reported due to the derived-to-base conversions.
// IWYU: Struct is...*-i1.h
// IWYU: Derived needs a declaration
// IWYU: Derived is...*-i2.h
// IWYU: operator<=> is...*-i1.h
static_assert(__builtin_lt_synthesizes_from_spaceship(Struct&, Derived&));

// Test that IWYU doesn't crash on a trait with dependent type.
template <class T>
void TplFn(const T&)
  requires(__builtin_lt_synthesizes_from_spaceship(T, int))
{
}

/**** IWYU_SUMMARY

tests/cxx/type_trait.cc should add these lines:
#include "tests/cxx/type_trait-i1.h"
#include "tests/cxx/type_trait-i2.h"

tests/cxx/type_trait.cc should remove these lines:
- class Class;  // lines XX-XX
- class StructDerivedClass;  // lines XX-XX
- class With3WayComp;  // lines XX-XX
- struct Struct;  // lines XX-XX
- template <typename> struct DeducibleTpl;  // lines XX-XX+1
- union Union1;  // lines XX-XX
- union Union2;  // lines XX-XX

The full include-list for tests/cxx/type_trait.cc:
#include "tests/cxx/type_trait-d1.h"  // for ClassConstRefProviding, ClassProviding, ClassRefProviding, DerivedProviding, DerivedPtrProviding, DerivedPtrRefProviding, DerivedRefProviding, Union1RefProviding, With3WayCompRefProviding
#include "tests/cxx/type_trait-d2.h"  // for BaseMemPtr, BaseNonProviding, ClassArray2NonProviding, ClassArray3NonProviding, ClassConstRefNonProviding, ClassNonProviding, ClassRefNonProviding, DerivedArrayNonProviding, DerivedMemPtr, DerivedNonProviding, DerivedPtrNonProviding, DerivedPtrRefNonProviding, DerivedRefNonProviding, Union1NonProviding, Union1PtrRefNonProviding, Union1RefNonProviding, UnionMemPtr, With3WayCompRefNonProviding
#include "tests/cxx/type_trait-i1.h"  // for Base, Class, DeducibleTpl, Enum (ptr only), Struct, StructDerivedClass, Union1, Union2, operator<, operator<=, operator<=>, operator>, operator>=
#include "tests/cxx/type_trait-i2.h"  // for Derived, With3WayComp

***** IWYU_SUMMARY */

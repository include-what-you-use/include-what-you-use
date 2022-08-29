//===--- enums.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test enumeration forward (opaque) declarations.

#include "tests/cxx/enums-d1.h"

// Test that IWYU considers this as a forward declaration
// which should be kept.
enum class DirectEnum5 : long;
// Test that opaque redeclaration should not lead to ignoring
// of IndirectEnum2::A use.
enum class IndirectEnum2 : int;

// For scoped and unscoped enums with fixed underlying type, opaque declaration
// is sufficient.
DirectEnum1 de1;
DirectEnum2 de2;
DirectEnum3 de3;
ns::DirectEnum4 de4;
DirectEnum5 de5;
// Test that 'bool' underlying type is represented as 'bool' in opaque
// declaration and not as '_Bool' clang builtin type.
DirectEnum6 de6;

// For an unscoped enum without explicitly specified underlying type, full
// definition is needed.
// IWYU: IndirectEnum1 is...*enums-i1.h
IndirectEnum1 ie1;
// When an enum item is mentioned, full definition of the enum is needed.
// IWYU: IndirectEnum2 is...*enums-i2.h
IndirectEnum2 ie2 = IndirectEnum2::A;
// In order to mention a nested enum, full type of the owning class is needed.
// IWYU: Struct1 is...*enums-i3.h
Struct1::IndirectEnum3 ie3;
// For unnamed enumeration, enumerators are reported.
// IWYU: UnnamedEnumItem2 is...*enums-i4.h
auto ie4 = UnnamedEnumItem2;
// Test that elaborated types don't replace the full definition for enums with
// underlying type not fixed, and an opaque declaration for enums with fixed
// underlying type.
// IWYU: IndirectEnum1 is...*enums-i1.h
enum IndirectEnum1* pe1;
// IWYU: IndirectEnum5 needs a declaration
enum IndirectEnum5 ie5;
// IWYU: IndirectEnum6 needs a declaration
enum IndirectEnum6 ie6;
// IWYU: IndirectEnum7 needs a declaration
enum IndirectEnum7 ie7;

// No need even for opaque declaration if enumeration or enumerator isn't named
// explicitly (the underlying type info should already be present in the
// translation unit, probably through transitive header inclusions). No matter,
// whether enum has fixed type or not, used in sizeof operator, pointer
// arithmetic or whatever.
auto p1 = &ie1 + sizeof(ie1);
auto p2 = &ie3 + sizeof(ie3);

template <typename T1, typename T2>
struct Template {
  T1 t1;
  T2 t2 = T2::B;
};

// Only IndirectEnum2 item is used inside Template specialization.
// IWYU: IndirectEnum2 is...*enums-i2.h
Template<DirectEnum1, IndirectEnum2> t;

struct Struct2 {
  // Test that IWYU doesn't suggest to remove this declaration.
  enum class Nested;
};

enum class Struct2::Nested { A, B, C };

// Some of function-local predefined variables allow to output enum item names.
// This trick is used in some enum reflection libraries (more about that is
// here: https://blog.rink.nu/2023/02/12/behind-the-magic-of-magic_enum). Test
// that IWYU detects such uses correctly and requires complete enum types
// so that item names are available to a compiler.

template <typename E, E>
auto GetPrettyFunctionStr1() {
  return __PRETTY_FUNCTION__;
}

template <typename E>
auto CallGetPrettyFunctionStr1() {
  return GetPrettyFunctionStr1<E, E{}>();
}

template <auto E>
auto GetPrettyFunctionStr2() {
  return __PRETTY_FUNCTION__;
}

template <typename E1, E1, typename E2, E2>
auto GetPrettyFunctionStr3() {
  return __PRETTY_FUNCTION__;
}

template <typename E>
auto GetPrettyFunctionStrNoEnumItem() {
  return __PRETTY_FUNCTION__;
}

template <typename E, E>
auto CallGetPrettyFunctionStrNoEnumItem() {
  return GetPrettyFunctionStrNoEnumItem<E>();
}

// Just to check that IWYU doesn't crash or something like that.
template <int>
auto GetPrettyFunctionNonEnum() {
  return __PRETTY_FUNCTION__;
}

template <auto E>
auto GetFuncStr() {
  return __func__;
}

template <auto E>
auto GetFunctionStr() {
  return __FUNCTION__;
}

void Fn() {
  constexpr IndirectEnum2 ie2 = {};
  // IWYU: IndirectEnum2 is...*enums-i2.h
  GetPrettyFunctionStr1<IndirectEnum2, ie2>();
  // IWYU: IndirectEnum2 is...*enums-i2.h
  CallGetPrettyFunctionStr1<IndirectEnum2>();
  // IWYU: IndirectEnum2 is...*enums-i2.h
  GetPrettyFunctionStr2<ie2>();
  // IWYU: IndirectEnum2 is...*enums-i2.h
  // IWYU: IndirectEnum8 needs a declaration
  // IWYU: IndirectEnum8 is...*enums-i2.h
  GetPrettyFunctionStr3<IndirectEnum2, ie2, IndirectEnum8, IndirectEnum8{}>();
  // IWYU: IndirectEnum9 needs a declaration
  // IWYU: IndirectEnum9 is...*enums-i2.h
  GetPrettyFunctionStr2<IndirectEnum9{}>();
  // Because IndirectEnum1 doesn't have a fixed underlying type, it is not
  // forward-declarable, hence g_ie1 should already provide it.
  // IWYU: g_ie1 is...*enums-i1.h
  GetPrettyFunctionStr2<g_ie1>();
  GetPrettyFunctionStrNoEnumItem<IndirectEnum2>();
  CallGetPrettyFunctionStrNoEnumItem<IndirectEnum2, IndirectEnum2{}>();
  GetPrettyFunctionNonEnum<1>();

  // Test other predefined variables.
  GetFuncStr<ie2>();
  GetFunctionStr<ie2>();
}

/**** IWYU_SUMMARY

tests/cxx/enums.cc should add these lines:
#include "tests/cxx/enums-i1.h"
#include "tests/cxx/enums-i2.h"
#include "tests/cxx/enums-i3.h"
#include "tests/cxx/enums-i4.h"
enum IndirectEnum6 : long;
enum class DirectEnum1;
enum class DirectEnum2 : int;
enum class DirectEnum6 : bool;
enum class IndirectEnum5;
enum struct DirectEnum3 : unsigned long long;
enum struct IndirectEnum7 : int;
namespace ns { enum DirectEnum4 : int; }

tests/cxx/enums.cc should remove these lines:
- #include "tests/cxx/enums-d1.h"  // lines XX-XX
- enum class IndirectEnum2 : int;  // lines XX-XX

The full include-list for tests/cxx/enums.cc:
#include "tests/cxx/enums-i1.h"  // for IndirectEnum1, g_ie1
#include "tests/cxx/enums-i2.h"  // for IndirectEnum2, IndirectEnum8, IndirectEnum9
#include "tests/cxx/enums-i3.h"  // for Struct1
#include "tests/cxx/enums-i4.h"  // for UnnamedEnumItem2
enum IndirectEnum6 : long;
enum class DirectEnum1;
enum class DirectEnum2 : int;
enum class DirectEnum5 : long;  // lines XX-XX
enum class DirectEnum6 : bool;
enum class IndirectEnum5;
enum class Struct2::Nested;  // lines XX-XX
enum struct DirectEnum3 : unsigned long long;
enum struct IndirectEnum7 : int;
namespace ns { enum DirectEnum4 : int; }

***** IWYU_SUMMARY */

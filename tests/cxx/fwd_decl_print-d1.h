//===--- fwd_decl_print-d1.h - test input file for iwyu -- C++ ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Simple tag types in global namespace.
enum Enum : int {};
class Class {};
struct Struct {};

// Simple tag types in user-defined namespace.
namespace ns1 {
enum EnumInNamespace : int {};
class ClassInNamespace {};
struct StructInNamespace {};
}

// Derived class types.
class DerivedClass : public Class {};

// Class types with attributes.
class [[nodiscard]] ClassWithStdAttr {};
class __attribute__((warn_unused_result)) ClassWithGNUAttr {};

// Class types with keyword attributes.
class alignas(8) ClassWithAlignAs {};
class alignas(alignof(bool)) ClassWithAlignAsExpr {};
class ClassWithFinal final {};

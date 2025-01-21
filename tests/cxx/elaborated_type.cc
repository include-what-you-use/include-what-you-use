//===--- elaborated_type.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that elaborated types are handled correctly.
//
// An elaborated type is either a type prefixed by type kind, e.g. 'class Foo',
// 'struct Bar' or 'enum Baz', or a namespace-qualified type.
// See C++ [dcl.type.elab].
//
// Test cases below have additional details on IWYU policy for these
// different elaborations.

#include "tests/cxx/elaborated_type_enum1.h"  // for ElaborationEnum1
#include "tests/cxx/elaborated_type_enum2.h"  // for ElaborationEnum2

class GlobalClass;

// Make sure both elaborated and bare enums require the full type.
void bare_enum(ElaborationEnum1 e);
void elaborated_enum(enum ElaborationEnum2 e);

// For C++ classes, a forward declaration should suffice for
// bare type names and nothing should be necessary for elaborated ones.
#include "tests/cxx/elaborated_type_class.h"

void bare_class(ElaborationClass* c);
void elaborated_class(class UnknownElaborationClass* c);

// Structs should work like classes.
#include "tests/cxx/elaborated_type_struct.h"

void bare_struct(ElaborationStruct* s);
void elaborated_struct(struct UnknownElaborationStruct* s);

// And unions.
#include "tests/cxx/elaborated_type_union.h"

void bare_union(ElaborationUnion* u);
void elaborated_union(union UnknownElaborationUnion* u);

// Namespace-qualified types must be forward-declared even
// if they are represented as elaborated types in Clang's AST,
// and the same goes for ::global-qualified types.
#include "tests/cxx/elaborated_type_namespace.h"

void namespace_qualified(Elaboration::Class* c);
void global_qualified(::GlobalClass* c);
void namespace_qualified_elab(class Elaboration::Class* c);
void global_qualified_elab(class ::GlobalClass* c);

// We can use elaborated types for templates, too, but
// they must also be forward-declared.
Elaboration::Template<int, float>* namespace_qualified_template;

// Elaborated types can be treated as forward-declarations, hence no fwd-decl is
// needed if a declaration is introduced by elaborated type. Implicit
// declaration introduction occurs at least when there isn't any previous
// declaration of the type in a translation unit.
void elaborated_first_elaborated(class FirstElaborated*);
void bare_first_elaborated(FirstElaborated*);
class FirstElaborated;
void previously_introduced(FirstElaborated*);

// But if an elaborated type appears after the first fwd-decl use of the type, a
// true forward declaration is still needed.
class FirstForwardDeclared;
void bare_first_forward_declared(FirstForwardDeclared*);
void elaborated_first_forward_declared(class FirstForwardDeclared*);

// Test that IWYU doesn't suggest to remove a line with an elaborated type as if
// there was a duplicating forward-declaration.
class ElaboratedInMultipleDecl *p1, *p2;
union {
  float a;
  int b;
} unnamed_union_obj1, unnamed_union_obj2;
struct {
} unnamed_struct_obj1, unnamed_struct_obj2;
struct NamedStruct {
} named_struct_obj1, named_struct_obj2;

/**** IWYU_SUMMARY

tests/cxx/elaborated_type.cc should add these lines:
class ElaborationClass;
namespace Elaboration { class Class; }
namespace Elaboration { template <typename T, typename U> struct Template; }
struct ElaborationStruct;
union ElaborationUnion;

tests/cxx/elaborated_type.cc should remove these lines:
- #include "tests/cxx/elaborated_type_class.h"  // lines XX-XX
- #include "tests/cxx/elaborated_type_namespace.h"  // lines XX-XX
- #include "tests/cxx/elaborated_type_struct.h"  // lines XX-XX
- #include "tests/cxx/elaborated_type_union.h"  // lines XX-XX
- class FirstElaborated;  // lines XX-XX

The full include-list for tests/cxx/elaborated_type.cc:
#include "tests/cxx/elaborated_type_enum1.h"  // for ElaborationEnum1
#include "tests/cxx/elaborated_type_enum2.h"  // for ElaborationEnum2
class ElaborationClass;
class FirstForwardDeclared;  // lines XX-XX
class GlobalClass;  // lines XX-XX
namespace Elaboration { class Class; }
namespace Elaboration { template <typename T, typename U> struct Template; }
struct ElaborationStruct;
union ElaborationUnion;

***** IWYU_SUMMARY */

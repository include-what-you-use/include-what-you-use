//===--- type_trait.c - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c23

// Tests handling the type trait __builtin_types_compatible_p.

#include "tests/c/type_trait-direct.h"

struct Indirect1;
typedef struct Indirect1 GlobalIndirect1NonProviding;
typedef struct Indirect1** GlobalIndirect1PtrPtrNonProviding;

// For tag types from the file scope, they should be just the same, no complete
// type needed.
static_assert(__builtin_types_compatible_p(struct Indirect1, struct Indirect1));
static_assert(__builtin_types_compatible_p(struct Indirect1*[],
                                           struct Indirect1*[]));
static_assert(__builtin_types_compatible_p(struct Indirect1(),
                                           struct Indirect1()));
static_assert(__builtin_types_compatible_p(int(struct Indirect1),
                                           int(struct Indirect1)));
static_assert(__builtin_types_compatible_p(_Atomic(struct Indirect1*),
                                           _Atomic(struct Indirect1*)));
// IWYU: Enum1 needs a declaration
static_assert(__builtin_types_compatible_p(enum Enum1, enum Enum1));
static_assert(!__builtin_types_compatible_p(struct Indirect1, int));
static_assert(!__builtin_types_compatible_p(int, struct Indirect1));
static_assert(!__builtin_types_compatible_p(struct Indirect1,
                                            struct Indirect2));
static_assert(!__builtin_types_compatible_p(struct Indirect1*,
                                            struct Indirect2*));
static_assert(!__builtin_types_compatible_p(struct Indirect1**,
                                            struct Indirect2**));

extern struct Indirect1 i1;
extern union Union u;
extern union DifferentTagKinds dtk;
// IWYU: Indirect1 is...*indirect.h
extern struct Indirect1 arr[5];
// IWYU: Indirect1 is...*indirect.h
extern _Atomic(struct Indirect1) atomic_arr[5];
// IWYU: Outer is...*indirect.h
extern _Atomic(struct Outer) atomic_outer_arr[];
// IWYU: Enum1 needs a declaration
enum Enum1 e1;
// IWYU: Enum2 needs a declaration
enum Enum2 e2;
// IWYU: EnumNonFixed is...*indirect.h
extern enum EnumNonFixed e3;
extern struct Outer o;
extern struct DifferentMemType dmt;
extern struct DifferentMemNumber dmnb;
extern struct DifferentMemName dmnm;
extern struct SelfReferring sr;
extern struct ContainsByVal cbv;
extern struct ContainsOuterByVal cobv;
extern struct ContainsAnonymousStruct cas;
extern struct DifferentAnonymousStruct das;
extern struct DifferentOrderWithAnonymous dowa;
extern struct DifferentAnonymousNumber dan;
extern struct ContainsAnonymousUnion cau;
extern struct ContainsUnnamedStruct cus;
extern struct DifferentUnnamedStructFieldName dusfn;
extern struct DifferentAnonymousKind dak;
extern struct ContainsUnnamedUnion cuu;
extern struct DifferentUnnamedKind duk;
extern struct ContainsUnnamedEnum cue;

// The same types, no complete type required.
static_assert(__builtin_types_compatible_p(struct Indirect1*[], typeof(i1)*[]));
static_assert(__builtin_types_compatible_p(struct Indirect1(), typeof(i1)()));
static_assert(__builtin_types_compatible_p(int(struct Indirect1),
                                           int(typeof(i1))));
static_assert(__builtin_types_compatible_p(_Atomic(struct Indirect1*),
                                           _Atomic(typeof(i1)*)));
// IWYU: Enum1 needs a declaration
static_assert(__builtin_types_compatible_p(enum Enum1, typeof(e1)));

void Fn() {
  // The same type, no complete type required.
  static_assert(__builtin_types_compatible_p(struct Indirect1, typeof(i1)));
  struct Indirect1 {
    int i;
  };
  typedef struct Indirect1* LocalIndirect1PtrArray[];
  // The global struct Indirect1 (i.e. the type of i1) required to check that it
  // is compatible (similar) to the local one.
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1, typeof(i1)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(i1), struct Indirect1));
  static_assert(!__builtin_types_compatible_p(struct Indirect2, typeof(i1)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(GlobalIndirect1NonProviding,
                                             struct Indirect1));
  static_assert(
      // IWYU: GlobalIndirect1Providing is...*indirect.h
      __builtin_types_compatible_p(GlobalIndirect1Providing, struct Indirect1));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1*, typeof(i1)*));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(i1)*, struct Indirect1*));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(GlobalIndirect1NonProviding*,
                                             struct Indirect1*));
  // IWYU: GlobalIndirect1Providing is...*indirect.h
  static_assert(__builtin_types_compatible_p(GlobalIndirect1Providing*,
                                             struct Indirect1*));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1*,
                                             GlobalIndirect1NonProviding*));
  static_assert(__builtin_types_compatible_p(
      struct Indirect1*,
      // IWYU: GlobalIndirect1Providing is...*indirect.h
      GlobalIndirect1Providing*));
  union Union {
    int i;
  };
  // IWYU: Union is...*indirect.h
  static_assert(__builtin_types_compatible_p(union Union, typeof(u)));
  struct DifferentTagKinds {
    struct Indirect1* obj;
  };
  static_assert(
      !__builtin_types_compatible_p(struct DifferentTagKinds, typeof(dtk)));
  // The trait ignores top-level cvr-qualifiers.
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1* const, typeof(i1)*));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect1 const*, typeof(i1)*));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect1*, volatile typeof(i1)*));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1 const*, typeof(i1) const*));
  static_assert(!__builtin_types_compatible_p(struct Indirect2*, typeof(i1)*));
  static_assert(!__builtin_types_compatible_p(struct Indirect1, typeof(i1)*));
  static_assert(!__builtin_types_compatible_p(struct Indirect1*, typeof(i1)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1**, typeof(i1)**));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(i1)**, struct Indirect1**));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(GlobalIndirect1PtrPtrNonProviding,
                                             struct Indirect1**));
  // IWYU: GlobalIndirect1PtrPtrProviding is...*indirect.h
  static_assert(__builtin_types_compatible_p(GlobalIndirect1PtrPtrProviding,
                                             struct Indirect1**));
  static_assert(!__builtin_types_compatible_p(struct Indirect1**, typeof(i1)*));
  static_assert(!__builtin_types_compatible_p(struct Indirect1*, typeof(i1)**));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect2**, typeof(i1)**));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1* [5], typeof(i1)* [5]));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect1* [5], typeof(i1)* [4]));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect2* [5], typeof(i1)* [5]));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1*[], typeof(i1)* [5]));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1* [5], typeof(i1)*[]));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct Indirect1*[], typeof(i1)*[]));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(typeof(i1)*[], struct Indirect1*[]));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect2*[], typeof(i1)*[]));
  // Element type cvr-qualifiers of top-level arrays are ignored as well.
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1* const[5],
                                             typeof(i1)* volatile[5]));
  static_assert(!__builtin_types_compatible_p(struct Indirect1* const(*)[5],
                                              typeof(i1)* volatile(*)[5]));
  // Test that applying a cvr-qualifier to an array type is identical
  // to applying it to the element type.
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(const LocalIndirect1PtrArray*,
                                             typeof(i1)* const(*)[]));
  static_assert(!__builtin_types_compatible_p(LocalIndirect1PtrArray*,
                                              typeof(i1)* const(*)[]));
  // No need to report global Indirect1 here because it is reported at arr
  // declaration.
  static_assert(__builtin_types_compatible_p(struct Indirect1[5], typeof(arr)));
  static_assert(__builtin_types_compatible_p(typeof(arr), struct Indirect1[5]));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Indirect1(), typeof(i1)()));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(i1)(), struct Indirect1()));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect1 (*)(), typeof(i1)()));
  static_assert(
      !__builtin_types_compatible_p(struct Indirect1(), typeof(i1)(int)));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(int(struct Indirect1), int(typeof(i1))));
  // The trait ignores parameter cvr-qualifiers but not the return type ones.
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1),
                                              const int(typeof(i1))));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int),
                                              int(typeof(i1))));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(struct Indirect1, int),
                                             int(typeof(i1), int)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int),
                                              int(typeof(i1), float)));
  static_assert(!__builtin_types_compatible_p(int(double, struct Indirect1),
                                              int(float, typeof(i1))));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(int, struct Indirect1*),
                                             int(int, typeof(i1)*)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(int, struct Indirect1*),
                                             int(const int, typeof(i1)*)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(
      int(int* restrict, struct Indirect1*), int(int*, typeof(i1)*)));
  // Array-to-pointer decay.
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(int*, struct Indirect1*),
                                             int(int[], typeof(i1)*)));
  // Function-to-pointer decay.
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(int (*)(), struct Indirect1*),
                                             int(int(), typeof(i1)*)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(struct Indirect1),
                                             int(GlobalIndirect1NonProviding)));
  static_assert(__builtin_types_compatible_p(
      int(struct Indirect1),
      // IWYU: GlobalIndirect1Providing is...*indirect.h
      int(GlobalIndirect1Providing)));
  static_assert(
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(
          int(struct Indirect1, struct Indirect1),
          // IWYU: GlobalIndirect1Providing is...*indirect.h
          int(GlobalIndirect1Providing, typeof(i1))));
  // IWYU: Indirect1 is...*indirect.h
  // IWYU: Union is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(struct Indirect1, union Union),
                                             int(typeof(i1), typeof(u))));
  // IWYU: Union is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(typeof(i1), union Union),
                                             int(typeof(i1), typeof(u))));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(_Atomic(struct Indirect1),
                                             _Atomic(typeof(i1))));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(_Atomic(struct Indirect1*),
                                             _Atomic(typeof(i1)*)));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(_Atomic(typeof(i1)*),
                                             _Atomic(struct Indirect1*)));
  // No need to report global Indirect1 here because it should be reported
  // at atomic_arr declaration.
  static_assert(__builtin_types_compatible_p(_Atomic(struct Indirect1)[5],
                                             typeof(atomic_arr)));
  // Enumerations are compatible with their underlying types regardless
  // of the enumeration content.
  // IWYU: Enum1 needs a declaration
  static_assert(__builtin_types_compatible_p(enum Enum1, int));
  // The same types, no enumeration content info is required.
  // IWYU: Enum1 needs a declaration
  static_assert(__builtin_types_compatible_p(enum Enum1, typeof(e1)));
  enum Enum1 : int { A1, B1, C1 };
  // The global enumeration content info is required here to determine that it
  // is identical to the content of the local Enum1.
  // IWYU: Enum1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(enum Enum1, typeof(e1)));
  // IWYU: Enum1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(e1), enum Enum1));
  // IWYU: Enum2 needs a declaration
  static_assert(!__builtin_types_compatible_p(enum Enum2, typeof(e1)));
  // IWYU: Enum1 is...*indirect.h
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(enum Enum1(struct Indirect1),
                                             typeof(e1)(typeof(i1))));
  enum Enum2 : int { A2, B2 };  // Different number of enumerators.
  // If the IWYU policy is to report Enum2 despite the differing content,
  // it should report the second parameter (Indirect1) as well here.
  // IWYU: Enum2 is...*indirect.h
  // IWYU: Indirect1 is...*indirect.h
  static_assert(!__builtin_types_compatible_p(int(enum Enum2, struct Indirect1),
                                              int(typeof(e2), typeof(i1))));
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(int(int, struct Indirect1),
                                             int(typeof(e2), typeof(i1))));
  enum EnumNonFixed { A3, B3, C3 };
  // No need to report the global EnumNonFixed here because its full definition
  // should be required at the e3 declaration.
  static_assert(__builtin_types_compatible_p(enum EnumNonFixed, typeof(e3)));
  static_assert(__builtin_types_compatible_p(typeof(e3), enum EnumNonFixed));

  // Test that the first parameters are not reported if the second ones differ.
  static_assert(!__builtin_types_compatible_p(
      // IWYU: Indirect2 needs a declaration
      int(union Union, struct Indirect2), int(typeof(u), typeof(i1))));
  static_assert(!__builtin_types_compatible_p(
      int(struct Indirect1, union Union), int(typeof(i1), int)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int),
                                              int(typeof(i1), union Union)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int),
                                              int(typeof(i1), int*)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int*),
                                              int(typeof(i1), int)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int (*)[]),
                                              int(typeof(i1), int*)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int*),
                                              int(typeof(i1), int (*)[])));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int (*)()),
                                              int(typeof(i1), int*)));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int*),
                                              int(typeof(i1), int (*)())));
  static_assert(!__builtin_types_compatible_p(int(struct Indirect1, int),
                                              int(typeof(i1), _Atomic(int))));
  static_assert(!__builtin_types_compatible_p(
      int(struct Indirect1, _Atomic(int)), int(typeof(i1), int)));

  // Test that the analysis recursively proceeds to the field types.
  struct Outer {
    int i;
    struct Indirect1* obj;
    enum Enum1** e;
  };
  // IWYU: Outer is...*indirect.h
  // IWYU: Indirect1 is...*indirect.h
  // IWYU: Enum1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct Outer*, typeof(o)*));
  // IWYU: Outer is...*indirect.h
  // IWYU: Indirect1 is...*indirect.h
  // IWYU: Enum1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(typeof(o)*, struct Outer*));

  struct DifferentMemType {
    int i;
    struct Indirect1* obj;
  };
  static_assert(
      !__builtin_types_compatible_p(struct DifferentMemType, typeof(dmt)));

  struct DifferentMemNumber {
    struct Indirect1* obj;
  };
  static_assert(
      !__builtin_types_compatible_p(struct DifferentMemNumber, typeof(dmnb)));

  struct DifferentMemName {
    struct Indirect1* obj;
    int k;
  };
  static_assert(
      !__builtin_types_compatible_p(struct DifferentMemName, typeof(dmnm)));

  struct SelfReferring {
    struct SelfReferring* p;
  };
  // IWYU: SelfReferring is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct SelfReferring, typeof(sr)));

  struct ContainsByVal {
    struct Indirect1 obj;
    enum Enum1 e;
  };
  // No need to report Indirect1 here because ContainsByVal definition should
  // provide it.
  static_assert(
      // IWYU: ContainsByVal is...*indirect.h
      // IWYU: Enum1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsByVal, typeof(cbv)));
  static_assert(
      // IWYU: ContainsByVal is...*indirect.h
      // IWYU: Enum1 is...*indirect.h
      __builtin_types_compatible_p(typeof(cbv), struct ContainsByVal));

  // Test that IWYU doesn't report the global Outer but proceeds with
  // the anaysis and reports the internals of Outer.
  struct ContainsOuterByVal {
    struct Outer o;
  };
  static_assert(
      // IWYU: ContainsOuterByVal is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      // IWYU: Enum1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsOuterByVal, typeof(cobv)));
  // IWYU: Indirect1 is...*indirect.h
  // IWYU: Enum1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(_Atomic(struct Outer)[],
                                             typeof(atomic_outer_arr)));
  static_assert(
      // IWYU: GlobalOuterProviding is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      // IWYU: Enum1 is...*indirect.h
      __builtin_types_compatible_p(GlobalOuterProviding, struct Outer));

  struct ContainsAnonymousStruct {
    struct {
      struct Indirect1* obj;
    };
    int i;
  };
  // IWYU: ContainsAnonymousStruct is...*indirect.h
  // IWYU: Indirect1 is...*indirect.h
  static_assert(__builtin_types_compatible_p(struct ContainsAnonymousStruct,
                                             typeof(cas)));

  struct DifferentAnonymousStruct {
    struct Indirect1* obj;
    struct {
      int i;
    };
  };
  static_assert(!__builtin_types_compatible_p(struct DifferentAnonymousStruct,
                                              typeof(das)));

  struct DifferentOrderWithAnonymous {
    int i;
    struct {
      struct Indirect1* obj;
    };
  };
  static_assert(!__builtin_types_compatible_p(
      struct DifferentOrderWithAnonymous, typeof(dowa)));

  struct DifferentAnonymousNumber {
    struct {
      struct Indirect1* obj;
      int i;
    };
  };
  static_assert(!__builtin_types_compatible_p(struct DifferentAnonymousNumber,
                                              typeof(dan)));

  struct ContainsAnonymousUnion {
    union {
      struct Indirect1* obj;
    };
  };
  static_assert(
      // IWYU: ContainsAnonymousUnion is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsAnonymousUnion, typeof(cau)));

  struct DifferentAnonymousKind {
    union {
      struct Indirect1* obj;
    };
  };
  static_assert(!__builtin_types_compatible_p(struct DifferentAnonymousKind,
                                              typeof(dak)));

  // NOTE: handling of unnamed entities in compilers is different and may
  // change. See the issues https://github.com/llvm/llvm-project/issues/151394
  // and https://github.com/llvm/llvm-project/issues/151735.
  struct ContainsUnnamedStruct {
    struct {
      struct Indirect1* obj;
    } field;
  };
  static_assert(
      // IWYU: ContainsUnnamedStruct is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsUnnamedStruct, typeof(cus)));

  struct DifferentUnnamedStructFieldName {
    struct {
      struct Indirect1* obj;
    } other_name;
  };
  static_assert(!__builtin_types_compatible_p(
      struct DifferentUnnamedStructFieldName, typeof(dusfn)));

  struct ContainsUnnamedUnion {
    union {
      struct Indirect1* obj;
    } field;
  };
  static_assert(
      // IWYU: ContainsUnnamedUnion is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsUnnamedUnion, typeof(cuu)));

  struct DifferentUnnamedKind {
    union {
      struct Indirect1* obj;
    } field;
  };
  static_assert(
      !__builtin_types_compatible_p(struct DifferentUnnamedKind, typeof(duk)));

  struct ContainsUnnamedEnum {
    enum { A, B } e;
    struct Indirect1* obj;
  };
  static_assert(
      // IWYU: ContainsUnnamedEnum is...*indirect.h
      // IWYU: Indirect1 is...*indirect.h
      __builtin_types_compatible_p(struct ContainsUnnamedEnum, typeof(cue)));
}

/**** IWYU_SUMMARY

tests/c/type_trait.c should add these lines:
#include "tests/c/type_trait-indirect.h"

tests/c/type_trait.c should remove these lines:
- #include "tests/c/type_trait-direct.h"  // lines XX-XX
- struct Indirect1;  // lines XX-XX

The full include-list for tests/c/type_trait.c:
#include "tests/c/type_trait-indirect.h"  // for ContainsAnonymousStruct, ContainsAnonymousUnion, ContainsByVal, ContainsOuterByVal, ContainsUnnamedEnum, ContainsUnnamedStruct, ContainsUnnamedUnion, Enum1, Enum2, EnumNonFixed, GlobalIndirect1Providing, GlobalIndirect1PtrPtrProviding, GlobalOuterProviding, Indirect1, Indirect2 (ptr only), Outer, SelfReferring, Union

***** IWYU_SUMMARY */

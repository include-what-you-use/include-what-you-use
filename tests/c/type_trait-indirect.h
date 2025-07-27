//===--- type_trait-indirect.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct Indirect1 {
  int i;
};

struct Indirect2 {
  int i;
};

union Union {
  int i;
};

union DifferentTagKinds {
  struct Indirect1* obj;
};

typedef struct Indirect1 GlobalIndirect1Providing;
typedef struct Indirect1** GlobalIndirect1PtrPtrProviding;

enum Enum1 : int { A1, B1, C1 };

enum Enum2 : int { A2, B2, C2 };

enum EnumNonFixed { A3, B3, C3 };

struct Outer {
  int i;
  struct Indirect1* obj;
  enum Enum1** e;
};

typedef struct Outer GlobalOuterProviding;

struct DifferentMemType {
  double i;
  struct Indirect1* obj;
};

struct DifferentMemNumber {
  struct Indirect1* obj;
  int i;
};

struct DifferentMemName {
  struct Indirect1* obj;
  int i;
};

struct SelfReferring {
  struct SelfReferring* p;
};

struct ContainsByVal {
  struct Indirect1 obj;
  enum Enum1 e;
};

struct ContainsOuterByVal {
  struct Outer o;
};

struct ContainsAnonymousStruct {
  struct {
    struct Indirect1* obj;
  };
  int i;
};

struct DifferentAnonymousStruct {
  struct {
    struct Indirect1* obj;
  };
  int i;
};

struct DifferentOrderWithAnonymous {
  struct {
    struct Indirect1* obj;
  };
  int i;
};

struct DifferentAnonymousNumber {
  struct {
    struct Indirect1* obj;
  };
  struct {
    int i;
  };
};

struct ContainsAnonymousUnion {
  union {
    struct Indirect1* obj;
  };
};

struct DifferentAnonymousKind {
  struct {
    struct Indirect1* obj;
  };
};

struct ContainsUnnamedStruct {
  struct {
    struct Indirect1* obj;
  } field;
};

struct DifferentUnnamedStructFieldName {
  struct {
    struct Indirect1* obj;
  } field;
};

struct ContainsUnnamedUnion {
  union {
    struct Indirect1* obj;
  } field;
};

struct DifferentUnnamedKind {
  struct {
    struct Indirect1* obj;
  } field;
};

struct ContainsUnnamedEnum {
  enum { A, B } e;
  struct Indirect1* obj;
};

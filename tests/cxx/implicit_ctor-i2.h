//===--- implicit_ctor-i2.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct IndirectWithImplicitCtor {
  // This is the implicit ctor.
  IndirectWithImplicitCtor(int x) {}
};

struct NoAutocastCtor {
  NoAutocastCtor();
  NoAutocastCtor(int, int);
};

struct NoTrivialCtorDtor {
  NoTrivialCtorDtor();
  ~NoTrivialCtorDtor();
};

struct MultipleRedeclStruct {
  MultipleRedeclStruct(int);
};

template <typename T>
struct ImplicitCtorInPartial;

template <typename T>
struct ImplicitCtorInPartial<T*> {
  ImplicitCtorInPartial(int) {
  }
};

struct InnerAggregate1 {};

struct OuterAggregate1 {
  InnerAggregate1 inner;
};

struct InnerAggregate2 {
  int i;
  int j;
};

struct OuterAggregate2 {
  int i;
  InnerAggregate2 inner;
  int j;
};

struct OuterAggregateWithRef {
  const InnerAggregate1& inner_ref;
};

struct I2NonAggregate {
  I2NonAggregate(int = 0);
};

struct AggregateWithNonAggregate {
  I2NonAggregate na;
};

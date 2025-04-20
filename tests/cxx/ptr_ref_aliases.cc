//===--- ptr_ref_aliases.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/ptr_ref_aliases-d1.h \
//            -Xiwyu --check_also=tests/cxx/ptr_ref_aliases-d2.h \
//            -I .

// Tests various cases of type alias usages when the underlying type is
// a pointer or a reference. Particularly, tests that `DerefKind` parameter
// value is correct.

#include "tests/cxx/ptr_ref_aliases-d1.h"
#include "tests/cxx/ptr_ref_aliases-d2.h"
#include "tests/cxx/ptr_ref_aliases-d3.h"
#include <typeinfo>

NonProvidingPtrAlias GetPtr();
NonProvidingRefAlias GetRef();

NonProvidingPtrAlias ptr_nonprovided = GetPtr();
NonProvidingRefAlias ref_nonprovided = GetRef();

ProvidingPtrAlias ptr_provided = GetPtr();
ProvidingRefAlias ref_provided = GetRef();

struct Base {
  virtual IndirectBase* RetPtr1();
  virtual IndirectBase& RetRef1();
  virtual IndirectBase* RetPtr2();
  virtual IndirectBase& RetRef2();
};

struct Derived : Base {
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  NonProvidingPtrAlias RetPtr1() override;
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  NonProvidingRefAlias RetRef1() override;
  ProvidingPtrAlias RetPtr2() override;
  ProvidingRefAlias RetRef2() override;
};

void Fn() {
  (void)sizeof(NonProvidingPtrAlias);
  (void)sizeof(ptr_nonprovided);
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)sizeof(NonProvidingRefAlias);
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)sizeof(ref_nonprovided);
  (void)sizeof(ProvidingRefAlias);
  (void)sizeof(ref_provided);

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  ptr_nonprovided->Method();
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  ref_nonprovided.Method();
  ptr_provided->Method();
  ref_provided.Method();

  try {
    // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  } catch (NonProvidingPtrAlias) {
    // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  } catch (NonProvidingRefAlias) {
  }
  try {
  } catch (ProvidingPtrAlias) {
  } catch (ProvidingRefAlias) {
  }

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  for (auto i : ref_nonprovided)
    ;
  for (auto i : ref_provided)
    ;

  IndirectBase* pB = nullptr;
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)static_cast<NonProvidingPtrAlias>(pB);
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)static_cast<NonProvidingRefAlias>(*pB);
  (void)static_cast<ProvidingPtrAlias>(pB);
  (void)static_cast<ProvidingRefAlias>(*pB);

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)ptr_nonprovided[1];
  (void)ptr_provided[1];

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)(ptr_nonprovided + 1);
  (void)(ptr_provided + 1);

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  static_assert(__is_convertible_to(NonProvidingPtrAlias, IndirectBase*));
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  static_assert(__is_convertible_to(NonProvidingRefAlias, IndirectBase&));
  static_assert(__is_convertible_to(ProvidingPtrAlias, IndirectBase*));
  static_assert(__is_convertible_to(ProvidingRefAlias, IndirectBase&));

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  ref_nonprovided << 1;
  ref_provided << 1;

  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  delete ptr_nonprovided;
  delete ptr_provided;

  int VarArgFn(...);
  // It is important to keep VarArgFn calls in unevaluated context, otherwise
  // CXXConstructExpr nodes appear which should be handled likewise other
  // implicit construction cases. Btw, clang accepts such code without complete
  // argument type, but gcc doesn't.
  (void)sizeof(VarArgFn(ptr_nonprovided));
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)sizeof(VarArgFn(ref_nonprovided));
  (void)sizeof(VarArgFn(ptr_provided));
  (void)sizeof(VarArgFn(ref_provided));

  const std::type_info& type_info1 = typeid(NonProvidingPtrAlias);
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  const std::type_info& type_info2 = typeid(NonProvidingRefAlias);
  const std::type_info& type_info3 = typeid(ProvidingPtrAlias);
  const std::type_info& type_info4 = typeid(ProvidingRefAlias);

  decltype(ptr_nonprovided) dpn;
  decltype(ref_nonprovided) drn = GetRef();
}

namespace ns {
using ::NonProvidingPtrAlias;
using ::NonProvidingRefAlias;
using ::ProvidingPtrAlias;
using ::ProvidingRefAlias;
}  // namespace ns

ns::NonProvidingPtrAlias ptr_nonprovided_with_using = GetPtr();
ns::NonProvidingRefAlias ref_nonprovided_with_using = GetRef();

ns::ProvidingPtrAlias ptr_provided_with_using = GetPtr();
ns::ProvidingRefAlias ref_provided_with_using = GetRef();

void TestSomeComplexIndirectionCases() {
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)sizeof(NonProvidingDoubleRefAlias);

  NonProvidingRefPtrAlias GetPtrRef();
  NonProvidingDoubleRefPtrAlias ptr_ref = GetPtrRef();
  // IWYU: Indirect is...*ptr_ref_aliases-i2.h
  (void)ptr_ref[1];
  NonProvidingDoublePtrAlias ptr_ptr = nullptr;
  (void)ptr_ptr[1];
}

/**** IWYU_SUMMARY

tests/cxx/ptr_ref_aliases.cc should add these lines:
#include "tests/cxx/ptr_ref_aliases-i2.h"
struct IndirectBase;

tests/cxx/ptr_ref_aliases.cc should remove these lines:
- #include "tests/cxx/ptr_ref_aliases-d3.h"  // lines XX-XX

The full include-list for tests/cxx/ptr_ref_aliases.cc:
#include <typeinfo>  // for type_info
#include "tests/cxx/ptr_ref_aliases-d1.h"  // for NonProvidingDoublePtrAlias, NonProvidingDoubleRefAlias, NonProvidingDoubleRefPtrAlias, NonProvidingPtrAlias, NonProvidingRefAlias, NonProvidingRefPtrAlias
#include "tests/cxx/ptr_ref_aliases-d2.h"  // for ProvidingPtrAlias, ProvidingRefAlias
#include "tests/cxx/ptr_ref_aliases-i2.h"  // for Indirect
struct IndirectBase;

***** IWYU_SUMMARY */

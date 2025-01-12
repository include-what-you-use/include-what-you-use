//===--- reexport_overridden-d1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// In order for a method to be overridden, its base class must already be
// included, which must in itself be self-sufficient. Thus, the function
// signature types should generally not trigger IWYU warnings here.

#include "tests/cxx/reexport_overridden-i1.h"

// IWYU should suggest to remove these fwd-decls:
class FwdAutoconvParam;
template <typename>
class IndirectTemplate;

class Derived1 : public Base {
 public:
  // Return type fully defined in Base.
  IndirectClass GetIndirect() override;

  // Return types forward-declared in Base.
  FwdRetType ReturnType() override;
  FwdRetType ReturnTypeUnusedInBody() override;

  // No need to forward-declare the return type even when it is aliased in Base.
  IndirectClass ReturnAliasedInBase() override;

  IndirectTemplate<FwdRetType> ReturnTemplate() override;

  // The using-declaration should be reported but the forward-declaration
  // should not.
  // IWYU: ns::FwdRetType is...*reexport_overridden-i3.h.*for using decl
  ns::FwdRetType ReturnPulledInOtherNSInDerived() override;

  // Unused argument available from Base.
  void ArgumentUnused(const IndirectClass& x) override;

  // Unused argument available from Base, in inline method.
  void ArgumentUnusedInline(const IndirectClass& x) override {
  }

  // The types fully used by inline definitions should be reported.
  // IWYU: Struct is...*reexport_overridden-i3.h
  void ArgumentByValueInline(Struct) override {
  }

  // Parameter with implicit conversion forward-declared in Base.
  void TakeFwdAutoconvParam(FwdAutoconvParam) override;

  // Type alias declaration should be reported here (moreover, it is not used
  // in the declaration from Base).
  // IWYU: Alias is...*reexport_overridden-i3.h
  void TakeNoConvParam(Alias) override;

  // Despite the declaration in Base doesn't mention IndirectClass explicitly,
  // its (forward-)declaration should nevertheless be already accessible.
  void TakeAliasedInBaseParam(IndirectClass) override;

  void TakeTemplate(IndirectTemplate<IndirectClass>) override;

  // IWYU: TemplatePtrAlias is...*reexport_overridden-i3.h
  void TakeAliasedTemplatePtr(TemplatePtrAlias) override;

  // The using-declaration should be reported but the forward-declaration
  // should not.
  // IWYU: ns::IndirectClass is...*reexport_overridden-i3.h.*for using decl
  void TakePulledInOtherNSInDerived(ns::IndirectClass) override;
};

class Derived2 : public Derived1 {
 public:
  IndirectClass GetIndirect() override;
  FwdRetType ReturnType() override;
  // It's better to report Alias here so that it may be replaced
  // by IndirectClass in Base without changing this file.
  // IWYU: Alias is...*reexport_overridden-i3.h
  Alias ReturnAliasedInBase() override;
  void TakeFwdAutoconvParam(FwdAutoconvParam) override;
  void TakeNoConvParam(IndirectClass) override;
};

/**** IWYU_SUMMARY
tests/cxx/reexport_overridden-d1.h should add these lines:
#include "tests/cxx/reexport_overridden-i3.h"

tests/cxx/reexport_overridden-d1.h should remove these lines:
- class FwdAutoconvParam;  // lines XX-XX
- template <typename> class IndirectTemplate;  // lines XX-XX+1

The full include-list for tests/cxx/reexport_overridden-d1.h:
#include "tests/cxx/reexport_overridden-i1.h"  // for Base
#include "tests/cxx/reexport_overridden-i3.h"  // for Alias, FwdRetType, IndirectClass, Struct, TemplatePtrAlias
***** IWYU_SUMMARY */

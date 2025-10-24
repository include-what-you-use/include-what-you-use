//===--- reexport_overridden-i1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"
#include "tests/cxx/reexport_overridden-i3.h"

// From reexport_overridden-i2.h
class FwdAutoconvParam;
class FwdRetType;

class Base {
 public:
  virtual IndirectClass GetIndirect();
  virtual FwdRetType ReturnType();
  virtual FwdRetType ReturnTypeUnusedInBody();
  virtual Alias ReturnAliasedInBase();
  virtual IndirectTemplate<FwdRetType> ReturnTemplate();
  virtual FwdRetType ReturnPulledInOtherNSInDerived();
  virtual void ArgumentUnused(const IndirectClass& x);
  virtual void ArgumentUnusedInline(const IndirectClass&);
  virtual void ArgumentByValueInline(Struct);
  virtual void TakeFwdAutoconvParam(FwdAutoconvParam);
  virtual void TakeNoConvParam(IndirectClass);
  virtual void TakeAliasedInBaseParam(Alias);
  virtual void TakeTemplate(IndirectTemplate<IndirectClass>);
  virtual void TakeSpecializedTemplate(IndirectTemplate<int>&);
  virtual void TakeAliasedTemplatePtr(TemplatePtrAlias);
  virtual void TakePulledInOtherNSInDerived(IndirectClass);
  virtual OuterTpl<IndirectClass>::Inner GetNestedInTpl();
};

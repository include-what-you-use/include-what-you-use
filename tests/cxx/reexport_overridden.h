//===--- reexport_overridden.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// In order for a method to be overridden, its base class must already be
// included, which must in itself be self-sufficient. Thus, the function
// signature types should not trigger IWYU warnings here.

#include "tests/cxx/reexport_overridden-d1.h"

class Derived : public Base {
 public:
  // Return type fully defined in Base.
  IndirectClass GetIndirect() override;

  // Return types forward-declared in Base.
  FwdRetType ReturnType() override;
  FwdRetType ReturnTypeUnusedInBody() override;

  // Unused argument available from Base.
  void ArgumentUnused(const IndirectClass& x) override;

  // Unused argument available from Base, in inline method.
  void ArgumentUnusedInline(const IndirectClass& x) override {
  }
};

/**** IWYU_SUMMARY

(tests/cxx/reexport_overridden.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

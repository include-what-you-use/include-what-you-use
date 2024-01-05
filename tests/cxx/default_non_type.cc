//===--- default_non_type.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a semi-reduced testcase from IWYU itself (iwyu.cc:main) where
// analysis of std::make_unique<> triggers an assertion failure in LLVM via
// GetDefaultedArgResugarMap. Fails with:
//
//   /usr/lib/llvm-18/include/clang/AST/TemplateBase.h:299:
//     QualType clang::TemplateArgument::getAsType() const: Assertion
//       `getKind() == Type && "Unexpected kind"' failed.

// IWYU_XFAIL

#include <memory>

struct ToolChain;

struct IwyuAction {
  explicit IwyuAction(const ToolChain&);
};

void foo(const ToolChain& t) {
  std::make_unique<IwyuAction>(t);
}

/**** IWYU_SUMMARY

(tests/cxx/default_non_type.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

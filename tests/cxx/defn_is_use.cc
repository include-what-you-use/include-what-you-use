//===--- defn_is_use.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "defn_is_use-decl.h"
#include "defn_is_use-namespace.h"

void ns1::ns2::Foo() {
}

void SomeFunction() {
}

/**** IWYU_SUMMARY

(tests/cxx/defn_is_use.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

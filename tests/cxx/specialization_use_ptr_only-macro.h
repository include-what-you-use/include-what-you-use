//===--- specialization_use_ptr_only-macro.h - test input -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_ptr_only-plain.h"
#include "tests/cxx/specialization_use_ptr_only-root.h"

// This forward-declaration makes the expansions of the macro below
// responsible for the uses of Plain in Holder<Plain>, and each of those uses
// reports a use of the forward-declaration here, which the definition in
// -plain.h satisfies.
struct Plain;

struct UsesOther {
  Other o1, o2, o3, o4, o5, o6, o7, o8, o9, o10, o11, o12;
};

#define MAKE_HOLDER(name) Holder<Plain> name;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_ptr_only-macro.h should add these lines:

tests/cxx/specialization_use_ptr_only-macro.h should remove these lines:
- struct Plain;  // lines XX-XX

The full include-list for tests/cxx/specialization_use_ptr_only-macro.h:
#include "tests/cxx/specialization_use_ptr_only-plain.h"  // for Other, Plain (pt...
#include "tests/cxx/specialization_use_ptr_only-root.h"  // for Holder

***** IWYU_SUMMARY */

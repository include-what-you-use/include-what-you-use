//===--- template_specialization.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/template_specialization-d2.h"
#include "tests/cxx/template_specialization-d3.h"

// When a specialization requires a forward-declaration of the primary template,
// a fwd-decl in the current file should be preferred to one in the
// otherwise-unused include -d2.h.
// The type alias is needed to trigger a full-use report because there is no
// forward-declaration in the alias-defining file. That full use is then
// recategorized to fwd-decl use because the defn is actually after the alias.
// IWYU: FwdDeclaredTpl needs a declaration
using FwdDeclaredTplSpecAlias = FwdDeclaredTpl<1>;
template <>
// IWYU: FwdDeclaredTpl needs a declaration
class FwdDeclaredTpl<1> {};

// IWYU: DefinedBeforeSpec needs a declaration
using DefinedBeforeSpecAlias = DefinedBeforeSpec<1>;
// Define the primary template; no diagnostic here.
template <int>
class DefinedBeforeSpec {};

/**** IWYU_SUMMARY

tests/cxx/template_specialization.h should add these lines:
template <int> class DefinedBeforeSpec;
template <int> class FwdDeclaredTpl;

tests/cxx/template_specialization.h should remove these lines:
- #include "tests/cxx/template_specialization-d2.h"  // lines XX-XX
- #include "tests/cxx/template_specialization-d3.h"  // lines XX-XX

The full include-list for tests/cxx/template_specialization.h:
template <int> class DefinedBeforeSpec;
template <int> class FwdDeclaredTpl;

***** IWYU_SUMMARY */

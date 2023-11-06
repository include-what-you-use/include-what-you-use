//===--- expl_inst_select.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This checks deduplication/selection of explicit template instantiations.

// IWYU_ARGS: -I .

#include "tests/cxx/expl_inst_select-d1.h"

// An explicit instantiation definition anchors a prior declaration.
// IWYU: Template is...*expl_inst_select-i1.h
// IWYU: Template is...*expl_inst_select-i2.h.*for explicit instantiation
template class Template<char>;

// An explicit instantiation declaration for later use.
// IWYU: Template is...*expl_inst_select-i1.h
extern template class Template<int>;

// Use of an explicit instantiation for which there is both a declaration and
// definition in the include closure should prefer a declaration.
// IWYU: Template is...*expl_inst_select-i1.h
// IWYU: Template is...*expl_inst_select-i2.h.*for explicit instantiation
Template<short> ts;

// Use of an explicit instantiation for which there is only a definition.
// IWYU: Template is...*expl_inst_select-i1.h
// IWYU: Template is...*expl_inst_select-i3.h.*for explicit instantiation
Template<double> td;

// No 'for explicit instantiation' diagnostic for use of an instantiation
// definition available in the same file.
// IWYU: Template is...*expl_inst_select-i1.h
Template<char> tc;

// No 'for explicit instantiation' diagnostic for use of an instantiation
// declaration available in the same file.
// IWYU: Template is...*expl_inst_select-i1.h
Template<int> ti;

/**** IWYU_SUMMARY

tests/cxx/expl_inst_select.cc should add these lines:
#include "tests/cxx/expl_inst_select-i1.h"
#include "tests/cxx/expl_inst_select-i2.h"
#include "tests/cxx/expl_inst_select-i3.h"

tests/cxx/expl_inst_select.cc should remove these lines:
- #include "tests/cxx/expl_inst_select-d1.h"  // lines XX-XX

The full include-list for tests/cxx/expl_inst_select.cc:
#include "tests/cxx/expl_inst_select-i1.h"  // for Template
#include "tests/cxx/expl_inst_select-i2.h"  // for Template
#include "tests/cxx/expl_inst_select-i3.h"  // for Template

***** IWYU_SUMMARY */

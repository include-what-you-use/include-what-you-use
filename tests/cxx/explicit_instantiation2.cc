//===--- explicit_instantiation2.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "explicit_instantiation-template_direct.h"
#include "explicit_instantiation2-template_helpers.h"
#include "explicit_instantiation2-template_short_direct.h"

// These tests verify that a dependent explicit instantiation declaration is
// always required when the main template is also needed, but not otherwise.
//
// Positive scenarios:
// 1a/1b: Implicit instantiation before and after an explicit instantiation
//        definition (2), as it affects the semantics.
// 2: Explicit instantiation definition.
// 3: Full use in a template, provided as an explicit parameter.
// 5: Full use in a template, provided as an default parameter.
// 7: Full use in a template, provided as a template template parameter.
// 8: Fwd-decl use in a template, provided as a template template parameter.
//
// Negative scenarios, where the dependent template specialization is not
// required, or it does not provide an explicit instantiation:
// 4: Fwd-decl use in a template, provided as an explicit parameter.
// 6: Fwd-decl use in a template, provided as a default parameter.
// 9: Implicit instantiation of Template<int>
// 10: Specialization of Template<T>
// 11: Explicit instantiation with a dependent instantiation declaration, but
//     provided by the template helper itself.


// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: Template is...*template_short.h.*for explicit instantiation
Template<short> t1a; // 1a

// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: Template is...*template_short.h.*for explicit instantiation
template class Template<short>;  // 2

// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: Template is...*template_short.h.*for explicit instantiation
Template<short> t1b; // 1b

// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: Template is...*template_short.h.*for explicit instantiation
FullUseArg<
    // IWYU: Template needs a declaration...*
    Template<short>>
    // IWYU: Template is...*explicit_instantiation-template.h
    t3; // 3

FwdDeclUseArg<
    // IWYU: Template needs a declaration...*
    Template<short>>
    t4; // 4

// IWYU: Template is...*explicit_instantiation-template.h
// IWYU: Template is...*template_short.h.*for explicit instantiation
TemplateAsDefaultFull<> t5; // 5
TemplateAsDefaultFwd<> t6; // 6

// IWYU: Template is...*template_short.h.*for explicit instantiation
TemplateTemplateArgShortFull<
    // IWYU: Template is...*explicit_instantiation-template.h
    Template>
    t7; // 7

TemplateTemplateArgShortFwd<
    // IWYU: Template is...*explicit_instantiation-template.h
    Template>
    t8; // 8

// IWYU: Template is...*explicit_instantiation-template.h
Template<int> t9; // 9

// IWYU: Template needs a declaration...*
template <> class Template<char> {};

TemplateAsDefaultFull<char> t10; // 10

TemplateAsDefaultFullProvided<> t11; // 11

/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation2.cc should add these lines:
#include "explicit_instantiation-template.h"
#include "explicit_instantiation2-template_short.h"

tests/cxx/explicit_instantiation2.cc should remove these lines:
- #include "explicit_instantiation-template_direct.h"  // lines XX-XX
- #include "explicit_instantiation2-template_short_direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation2.cc:
#include "explicit_instantiation-template.h"  // for Template
#include "explicit_instantiation2-template_helpers.h"  // for FullUseArg, FwdDeclUseArg, TemplateAsDefaultFull, TemplateAsDefaultFullProvided, TemplateAsDefaultFwd, TemplateTemplateArgShortFull, TemplateTemplateArgShortFwd
#include "explicit_instantiation2-template_short.h"  // for Template

***** IWYU_SUMMARY */

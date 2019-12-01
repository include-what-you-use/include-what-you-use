//===--- explicit_instantiation.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "explicit_instantiation-template_direct.h"

// Test that all explicit instantiations variants of the base template
// require the full type:

// - Declaration and definition
// IWYU: Template is...*explicit_instantiation-template.h
extern template class Template<int>;
// IWYU: Template is...*explicit_instantiation-template.h
template class Template<int>;

// - Only declaration
// IWYU: Template is...*explicit_instantiation-template.h
extern template class Template<short>;

// - Only definition
// IWYU: Template is...*explicit_instantiation-template.h
template class Template<double>;


// The explicit instantiation of a specialization only needs a declaration
// of the base template
// IWYU: Template needs a declaration
template<> class Template<char> {};
extern template class Template<char>;


/**** IWYU_SUMMARY

tests/cxx/explicit_instantiation.cc should add these lines:
#include "explicit_instantiation-template.h"

tests/cxx/explicit_instantiation.cc should remove these lines:
- #include "explicit_instantiation-template_direct.h"  // lines XX-XX

The full include-list for tests/cxx/explicit_instantiation.cc:
#include "explicit_instantiation-template.h"  // for Template

***** IWYU_SUMMARY */

//===--- template_default_args_roundtrip.cc - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The real-world issue reduced in this test case is an implicit use of a
// specialization of std::hash, by way of a default template argument.
// For example:
//
//   class C {};
//
//   namespace std {
//   template<> struct hash<C> {
//     size_t operator()(C c) const {
//       return sizeof(c);
//     }
//   };
//   }
//
//   // The default for the unordered_set Hash type parameter is std::hash<C>
//   // from above.
//   std::unordered_set<C> s;


#include "template_default_args_roundtrip-template.h"

// Provides DefaultArgument<int> indirectly
#include "template_default_args_roundtrip-indirect.h"
// Provides DefaultArgument<int> directly
#include "template_default_args_roundtrip-direct.h"


//Template making a full use of DefaultArgument<int>
Template<int> templ;

/**** IWYU_SUMMARY

tests/cxx/template_default_args_roundtrip.cc should add these lines:

tests/cxx/template_default_args_roundtrip.cc should remove these lines:
- #include "template_default_args_roundtrip-indirect.h"  // lines XX-XX

The full include-list for tests/cxx/template_default_args_roundtrip.cc:
#include "template_default_args_roundtrip-direct.h"  // for DefaultArgument
#include "template_default_args_roundtrip-template.h"  // for Template

***** IWYU_SUMMARY */

//===--- 1250b.cc - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "1250b-direct.h"

template<class T>
// IWYU: Template...*needs a declaration
void func_template(Template<T>* v) {
  // IWYU: consume is...*1250b-i2.h
  consume(
    // IWYU: Template is...*1250b-i1.h
    v->begin(),
    // IWYU: Template is...*1250b-i1.h
    v->end());
}

void instantiate() {
  // IWYU: Template is...*1250b-i1.h
  Template<int> v;
  // BUG: consume() is reported here.
  func_template(&v);
}

// IWYU seems to miss the two calls to begin() and end() in the function
// template above, so Template is only reported for forward-decl use.

// For contrast, a plain function with instantiated template reports everything
// as expected. You can uncomment below to verify.

// // IWYU: Template...*needs a declaration
// void function(Template<int>* v) {
//   // Curiosity: this reports as consume(:0, :0), not just consume.
//   // IWYU: consume(:0, :0) is...*1250b-i2.h
//   consume(
//     // IWYU: Template is...*1250b-i1.h
//     v->begin(),
//     // IWYU: Template is...*1250b-i1.h
//     v->end());
// }

/**** IWYU_SUMMARY

tests/bugs/1250/1250b.cc should add these lines:
#include "1250b-i1.h"
#include "1250b-i2.h"

tests/bugs/1250/1250b.cc should remove these lines:
- #include "1250b-direct.h"  // lines XX-XX

The full include-list for tests/bugs/1250/1250b.cc:
#include "1250b-i1.h"  // for Template
#include "1250b-i2.h"  // for consume

***** IWYU_SUMMARY */

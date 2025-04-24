//===--- 228.hpp - test input file for iwyu -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <a.hpp>

class B : public A {
  public:
    void doSomethingMore();
};

/**** IWYU_SUMMARY

(tests/bugs/228/228.hpp has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

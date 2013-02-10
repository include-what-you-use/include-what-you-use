//===--- cvr.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cvr-derived.h" // for Derived
#include "tests/cvr-class.h"   // for Class

class Base;

class ReturnsBase {
  virtual Base* non_covariant() = 0;

  virtual Base* covariant_derived() = 0;
  virtual const Class* covariant_cv_qual() = 0;
};

class ReturnsDerived : public ReturnsBase {
  // Normal case, do not trigger covariant return types.
  // This should require only a forward declaration of Base.
  Base* non_covariant() {
    return 0;
  }

  // C++ [class.virtual]p7, second bullet
  // Trigger covariant return types:
  //  Base is an unambiguous and accessible direct or indirect base class of
  //  Derived.
  Derived* covariant_derived() {
    return 0;
  }

  // C++ [class.virtual]p7, third bullet
  // Trigger covariant return types:
  //  Class* has less cv-qualification than const Class*.
  Class* covariant_cv_qual() {
    return 0;
  }
};

/**** IWYU_SUMMARY

(tests/cvr.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

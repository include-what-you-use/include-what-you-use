//===--- template_args_assoc.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename>
struct Template {
  static const int static_var;
  virtual void var() const;
};

template <typename E>
const int Template<E>::static_var = 0;

template <typename E>
void Template<E>::var() const {
  (void)static_var;
}

/**** IWYU_SUMMARY

(tests/cxx/template_args_assoc.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

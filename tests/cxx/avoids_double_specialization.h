//===--- avoids_double_specialization.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The same as in the associated .cc-file. Typedefs may behave differently in
// source and header files.

template <class T>
struct HeaderFoo {
  static int statici;
};
template <class T>
int HeaderFoo<T>::statici = 0;

template <class T>
struct HeaderBar {
  typedef HeaderFoo<T> value;
};

inline HeaderFoo<float> implicit_header_foo;
typedef HeaderFoo<float> HeaderBaz;

inline HeaderBar<float> implicit_header_bar;

/**** IWYU_SUMMARY

(tests/cxx/avoids_double_specialization.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

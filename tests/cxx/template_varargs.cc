//===--- template_varargs.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a testcase heavily reduced from the most representative
// Boost.Serialization example available online:
// https://stackoverflow.com/a/33226687/96963.
//
// This input would previously crash IWYU because the implicit constructor of
// __builtin_va_list inside the virtual member function in a template does not
// have any location information.

template <class T>
class Varargs {
  // Virtual member function is traversed as part of instantiation.
  virtual void* unused(unsigned, ...) const {
    __builtin_va_list x;
    return nullptr;
  }
};

template <class T>
struct Creator {
  static T make() {
    return T();
  }
};

// Necessary complications to instantiate Varargs in a deeply nested context.
struct Base {
  Base(const Varargs<int>&);
};

template <class T>
struct Derived : Base {
  Derived() : Base(Creator<Varargs<int>>::make()) {
  }
};

template <class T>
void InstantiateDerived() {
  // Only instantiate the template, don't call the method.
  (void)Creator<Derived<T>>::make;
}

void p() {
  InstantiateDerived<int>();
}

/**** IWYU_SUMMARY

(tests/cxx/template_varargs.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

// IWYU_XFAIL

#include "958.h"
#include "file1.h"

template <typename T>
int templFunc(T val) {
  return val.x;
}

// Explicit function template instantiation:
// https://en.cppreference.com/w/cpp/language/function_template#Explicit_instantiation
template int templFunc<X>(X val);

/**** IWYU_SUMMARY

(tests/bugs/958/958.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

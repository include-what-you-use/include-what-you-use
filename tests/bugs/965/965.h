#include "a.h"

template <typename T>
int size() {
  return sizeof(T);
}
template <int X>
int foo() {
  return X + size<Foo>();
}

/**** IWYU_SUMMARY

(tests/bugs/965/965.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

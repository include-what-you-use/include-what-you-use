// IWYU_XFAIL

#include "a.h"

struct Bar {
  Foo* f;
};

template <typename T>
int baz(const T& t) {
  return t.f->foo;
}

int bazz() {
  Bar b;
  return baz(b);
}

/**** IWYU_SUMMARY

(tests/bugs/973/973.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

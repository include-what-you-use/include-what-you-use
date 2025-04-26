// IWYU_XFAIL

#include "bar.h"
template <typename T>
void do_something() {
  Foo<T> f;
}

void baz() {
  do_something<int>();
}

/**** IWYU_SUMMARY

(tests/bugs/422/422.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

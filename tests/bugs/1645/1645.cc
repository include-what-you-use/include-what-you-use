// IWYU_XFAIL
#include "bar.h"

int main() {
  bar();
  foo_t foo{};
}

/**** IWYU_SUMMARY

(tests/bugs/1645/1645.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

// IWYU_XFAIL
#include <cmath>

static void f() {
  (void)std::abs(-12);
}

/**** IWYU_SUMMARY

(tests/bugs/1000/1000.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

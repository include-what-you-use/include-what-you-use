// IWYU_XFAIL

#include "s.h"

// #include <unordered_set>

void f(const S& s) {
  // No diagnostic expected.
  for (int i : s.i) {
  }
}

/**** IWYU_SUMMARY

(tests/bugs/1311/1311.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

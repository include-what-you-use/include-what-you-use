// IWYU_XFAIL
#include "b.h"

int f() {
  MyData data{};
  return data.meters.value;
}

/**** IWYU_SUMMARY

(tests/bugs/901/901.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

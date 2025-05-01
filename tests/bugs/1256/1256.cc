// IWYU_XFAIL

#include "1256.h"
#include "results.h"

static auto f() {
  return results{};
}

static auto g() {
  funcs fs = {.get_results = &f};
}

/**** IWYU_SUMMARY

(tests/bugs/1256/1256.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

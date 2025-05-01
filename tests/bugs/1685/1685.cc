// IWYU_XFAIL
#include "buffer.h"

struct Foo {
  Buffer<float> x;
};

/**** IWYU_SUMMARY

(tests/bugs/1685/1685.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

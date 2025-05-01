// IWYU_XFAIL
#include "inner.h"
#include "outer.h"

bool foo(Outer *outer) {
    return outer->inner->w == 123;
}

/**** IWYU_SUMMARY

(tests/bugs/1665/1665.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

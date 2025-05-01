// IWYU_XFAIL

#include "container.h"

int foo(Container& c) {
    c.push_back(5);
    auto it = c.begin();
    return *it;
}

/**** IWYU_SUMMARY

(tests/bugs/598/598.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

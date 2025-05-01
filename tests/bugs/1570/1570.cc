// IWYU_XFAIL

// IWYU does not understand forwarding headers.

#include "foo-fwd.h"         // Foo (forward)

// Forward suffices.
Foo *p;

/**** IWYU_SUMMARY

(tests/bugs/1570/1570.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
